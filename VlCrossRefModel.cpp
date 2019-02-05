/*
* Copyright 2018 Rochus Keller <mailto:me@rochus-keller.ch>
*
* This file is part of the Verilog parser library.
*
* The following is the license that applies to this copy of the
* library. For a license to use the library under conditions
* other than those described here, please email to me@rochus-keller.ch.
*
* GNU General Public License Usage
* This file may be used under the terms of the GNU General Public
* License (GPL) versions 2.0 or 3.0 as published by the Free Software
* Foundation and appearing in the file LICENSE.GPL included in
* the packaging of this file. Please review the following information
* to ensure GNU General Public Licensing requirements will be met:
* http://www.fsf.org/licensing/licenses/info/GPLv2.html and
* http://www.gnu.org/copyleft/gpl.html.
*/

#include "VlCrossRefModel.h"
#include "VlSynTree.h"
#include "VlErrors.h"
#include "VlIncludes.h"
#include "VlPpSymbols.h"
#include "VlPpLexer.h"
#include "VlParser.h"
#include "VlNumLex.h"
#include <QtDebug>
#include <QElapsedTimer>
#include <QThread>
#include <QBuffer>
using namespace Vl;

//#define _DUMP_AST

// für QtConcurrent::run braucht man extra eine separate Library mit minimalem Mehrwert
class CrossRefModel::Worker : public QThread
{
public:
    Worker(CrossRefModel* p):QThread(p){}
    void run()
    {
        //qDebug() << "CrossRefModel::Worker start";
        CrossRefModel* mdl = static_cast<CrossRefModel*>(parent());
        runUpdater(mdl);
        //qDebug() << "CrossRefModel::Worker end";
    }
};

CrossRefModel::CrossRefModel(QObject *parent, FileCache* fc) : QObject(parent)
{
    d_worker = new Worker(this);
    connect(d_worker,SIGNAL(finished()), this, SLOT(onWorkFinished()) );

    d_fcache = fc;

    d_incs = new Includes(this);
    d_incs->setAutoAdd(false);

    d_syms = new PpSymbols(this);
    // TEST e200
    /*
    d_syms->addSymbol( "DISABLE_SV_ASSERTION", "", Tok_String );
    d_syms->addSymbol( "E203_MTVEC_TRAP_BASE", "0", Tok_Natural_number );
    d_syms->addSymbol( "E203_ITCM_DATA_WIDTH_IS_32", "1", Tok_Natural_number );
    */

    d_errs = new Errors(this);
    d_errs->setShowWarnings(false);
}

CrossRefModel::~CrossRefModel()
{
    d_break = 1;
    d_worker->wait();
}

bool CrossRefModel::updateFiles(const QStringList& files, bool synchronous)
{
    d_lock.lockForWrite();
    foreach( const QString& f, files )
    {
        if( !d_work.contains(f) )
            d_work.append(f);
    }
    d_lock.unlock();

    if( !d_worker->isRunning() )
        d_worker->start();
    if( synchronous )
        d_worker->wait();
    return true;
}

bool CrossRefModel::parseString(const QString& code, const QString& sourcePath)
{
    d_lock.lockForWrite();

    ScopeRefList scopes;
    IfDefOutLists idols;
    SectionLists secs;

    Errors errs(0,true);
    errs.setShowWarnings(false);
    errs.setReportToConsole(false);
    errs.setRecord(true);

    QBuffer in;
    in.setData( code.toLatin1() );
    in.open(QIODevice::ReadOnly);
    parseStream( &in, sourcePath, scopes, idols, secs[sourcePath], &errs, d_syms, d_incs, d_fcache );
    insertFiles( QStringList() << sourcePath, scopes, idols, secs, &errs, false );

    d_lock.unlock();
    return errs.getErrCount() == 0;
}

void CrossRefModel::clear()
{
    QSet<QString> files;
    d_lock.lockForWrite();
    foreach( const QString& f, d_work )
        files.insert(f);
    d_work.clear();
    foreach( const SymRef& sub, d_global.d_children )
    {
        files.insert(sub->d_tok.d_sourcePath);
    }
    d_global.d_children.clear();
    d_global.d_names.clear();
    d_idols.clear();
    d_index.clear();
    d_revIndex.clear();
    d_lock.unlock();
    emit sigModelUpdated();
    foreach( const QString& file, files )
        emit sigFileUpdated(file);

}

bool CrossRefModel::isEmpty() const
{
    d_lock.lockForRead();
    const bool res = d_global.d_children.isEmpty();
    d_lock.unlock();
    return res;
}

static bool hitsArea( const CrossRefModel::SymRef& sub, quint32 line, quint16 col, const QString& source )
{
    // es wurde in isHit bereits verifiziert, dass sub selber nicht getroffen wurde
    if( sub->tok().d_sourcePath != source || sub->tok().d_lineNr > line ||
            ( sub->tok().d_lineNr == line && sub->tok().d_colNr > col ) )
        return false;
    if( sub->children().isEmpty() )
        return false;
    foreach( const CrossRefModel::SymRef& subsub, sub->children() )
    {
        if( subsub->tok().d_sourcePath == source &&
                ( line < subsub->tok().d_lineNr || line == subsub->tok().d_lineNr && col <= subsub->tok().d_colNr ) )
            return true;
    }
    return false;
}

CrossRefModel::TreePath CrossRefModel::findSymbolBySourcePos(const QString& file, quint32 line, quint16 col,
                                                             bool onlyIdents , bool hitEmpty) const
{
    SymRefList list;
    d_lock.lockForRead();
    list = d_global.children();
    d_lock.unlock();

    TreePath res;
    foreach( const SymRef& sub, list )
    {
        if( sub->d_tok.d_sourcePath == file )
        {
            res.push_front( sub );
            if( findSymbolBySourcePosImp( res,line,col,onlyIdents,hitEmpty) )
                return res;
            if( hitEmpty && hitsArea( sub, line, col, file ) )
                return res;
            res.pop_front();
        }
    }
    return res;
}

CrossRefModel::IdentDeclRef CrossRefModel::findDeclarationOfSymbolAtSourcePos(const QString& file, quint32 line, quint16 col) const
{
    TreePath path = findSymbolBySourcePos( file, line, col );
    if( path.isEmpty() )
        return IdentDeclRef();
    else
        return findDeclarationOfSymbol(path.first().data());
}

CrossRefModel::IdentDeclRef CrossRefModel::findDeclarationOfSymbol(const CrossRefModel::Symbol* sym) const
{
    d_lock.lockForRead();
    IdentDeclRef res(const_cast<IdentDecl*>(d_index.value(sym))); // es gibt eh nur const-Methoden
    d_lock.unlock();
    return res;
}

CrossRefModel::SymRefList CrossRefModel::findAllReferencingSymbols(const CrossRefModel::Symbol* sym) const
{
    SymRefList res;
    d_lock.lockForRead();
    QList<const Symbol*> tmp = d_revIndex.values( sym );
    foreach( const Symbol* s, tmp )
        res.append( SymRef(const_cast<Symbol*>(s)) );
    d_lock.unlock();
    return res;
}

CrossRefModel::SymRefList CrossRefModel::findReferencingSymbolsByFile(const CrossRefModel::Symbol* sym, const QString& file) const
{
    SymRefList res;
    d_lock.lockForRead();
    QList<const Symbol*> tmp = d_revIndex.values( sym );
    foreach( const Symbol* s, tmp )
    {
        if( s->d_tok.d_sourcePath == file )
            res.append( SymRef(const_cast<Symbol*>(s)) );
    }
    d_lock.unlock();
    return res;
}

CrossRefModel::IfDefOutList CrossRefModel::getIfDefOutsByFile(const QString& file) const
{
    IfDefOutList res;
    d_lock.lockForRead();
    res = d_idols.value(file);
    d_lock.unlock();
    return res;
}

CrossRefModel::SectionList CrossRefModel::getSections(const QString& file) const
{
    SectionList res;
    d_lock.lockForRead();
    res = d_sections.value(file);
    d_lock.unlock();
    return res;
}

CrossRefModel::Section CrossRefModel::findSectionBySourcePos(const QString& file, quint32 line, quint16 col) const
{
    SectionList l = getSections(file);
    foreach( const Section& t, l )
    {
        if( t.d_lineFrom == line ) // && t.d_colNr <= col )
            return t;
    }
    return Section();
}

CrossRefModel::SymRef CrossRefModel::findGlobal(const QByteArray& name) const
{
    SymRef res;
    d_lock.lockForRead();
    const CrossRefModel::IdentDecl* decl = findNameInScope( &d_global, name, false);
    if( decl )
        res = decl->decl();
    d_lock.unlock();
    return res;
}

CrossRefModel::SymRefList CrossRefModel::getGlobalSyms(const QString& file) const
{
    SymRefList res;
    d_lock.lockForRead();
    if( file.isEmpty() )
        res = d_global.d_children;
    else
    {
        foreach( const SymRef& sym, d_global.d_children )
        {
            if( sym->d_tok.d_sourcePath == file )
                res.append(sym);
        }
    }
    d_lock.unlock();
    return res;
}

CrossRefModel::IdentDeclRefList CrossRefModel::getGlobalNames(const QString& file) const
{
    IdentDeclRefList res;
    d_lock.lockForRead();

    Scope::Names::const_iterator i;
    for( i = d_global.d_names.begin(); i != d_global.d_names.end(); ++i )
    {
        if( file.isEmpty() || file == i.value()->tok().d_sourcePath )
            res.append( IdentDeclRef( i.value() ) );
    }
    d_lock.unlock();
    return res;
}

QList<Token> CrossRefModel::findTokenByPos(const QString& line, int col, int* pos)
{
    PpLexer l;
    l.setIgnoreAttrs(false);
    l.setPackAttrs(false);
    l.setIgnoreComments(false);
    l.setPackComments(false);

    QList<Token> res = l.tokens(line);
    if( pos )
    {
        *pos = -1;
        for( int i = 0; i < res.size(); i++ )
        {
            const Token& t = res[i];
            if( t.d_colNr <= col && col < ( t.d_colNr + t.d_len ) )
            {
                *pos = i;
            }
        }
    }
    return res;
}

QString CrossRefModel::qualifiedName(const CrossRefModel::TreePath& path, bool skipFirst)
{
    return qualifiedNameParts(path,skipFirst).join(QChar('.') );
}

QStringList CrossRefModel::qualifiedNameParts(const CrossRefModel::TreePath& path, bool skipFirst)
{
    QStringList res;
    for( int i = path.size() - 1; i >= 0; i-- )
    {
        QByteArray name = path[i]->d_tok.d_val;
        if( path[i]->d_tok.d_type == SynTree::R_module_or_udp_instantiation )
            name.clear();

        if( !name.isEmpty() && ( i != 0 || !skipFirst ) )
        {
            res << QString::fromLatin1( name );
        }
    }
    return res;
}

const CrossRefModel::Scope*CrossRefModel::closestScope(const CrossRefModel::TreePath& path)
{
    for( int i = 0; i < path.size(); i++ )
    {
        Q_ASSERT(path[i]);
        const Scope* s = path[i]->toScope();
        if( s )
            return s;
    }
    return 0;
}

const CrossRefModel::Branch*CrossRefModel::closestBranch(const CrossRefModel::TreePath& path)
{
    for( int i = 0; i < path.size(); i++ )
    {
        Q_ASSERT(path[i]);
        const Branch* s = path[i]->toBranch();
        if( s )
            return s;
    }
    return 0;
}

const CrossRefModel::IdentDecl* CrossRefModel::findNameInScope(const Scope* scope, const QByteArray& name, bool recursiv, bool ports)
{
    const IdentDecl* res = scope->d_names.value(name);
    if( res )
        return res;
    if( ports && scope->d_tok.d_type == SynTree::R_module_declaration )
    {
        const Scope* lop = Symbol::toScope( findFirst( scope, SynTree::R_list_of_ports) );
        if( lop )
        {
            res = lop->d_names.value(name);
            if( res )
                return res;
        }
    }
    if( scope->d_super && recursiv )
        return findNameInScope( scope->d_super->toScope(), name, recursiv, ports );
    else
        return 0;
}

static void findTokensOnSameLine( QList<const SynTree*>& res, const SynTree* st, int line, const QString& path )
{
    foreach( const SynTree* sub, st->d_children )
    {
        if( sub->d_tok.d_sourcePath == path && sub->d_tok.d_lineNr > quint32(line))
            return;
        if( sub->d_tok.d_type < SynTree::R_First )
            res << sub;
        else
            findTokensOnSameLine(res, sub, line, path );
    }
}

quint16 CrossRefModel::calcTextLenOfDecl(const SynTree* st)
{
    QList<const SynTree*> tokens;
    findTokensOnSameLine( tokens, st, st->d_tok.d_lineNr, st->d_tok.d_sourcePath );
    int i = 0;
    while( i < tokens.size() )
    {
        const SynTree* cur = tokens[i];
        switch( cur->d_tok.d_type )
        {
        case Tok_Semi:
        case Tok_Hash:
        case Tok_Lpar:
        case Tok_identifier:
            return cur->d_tok.d_colNr - st->d_tok.d_colNr;
        case Tok_Lbrack:
            {
                i++;
                int level = 1;
                while( i < tokens.size() )
                {
                    if( tokens[i]->d_tok.d_type == Tok_Rbrack )
                    {
                        level--;
                        if(level==0)
                            break;
                    }else if( tokens[i]->d_tok.d_type == Tok_Lbrack )
                        level++;
                    i++;
                }
            }
            break;
        default:
            break;
        }
        i++;
    }
    if( !tokens.isEmpty() && tokenIsReservedWord( tokens.first()->d_tok.d_type) )
        return ::strlen( tokenToString( tokens.first()->d_tok.d_type ) );
    else
        return 0;
}

quint16 CrossRefModel::calcKeyWordLen(const SynTree* st)
{
    if( !st->d_children.isEmpty() && tokenIsReservedWord( st->d_children.first()->d_tok.d_type ) )
        return ::strlen( tokenToString( st->d_children.first()->d_tok.d_type ) );
    else
        return 0;
}

static inline bool isHit( const CrossRefModel::SymRef& sub, quint32 line, quint16 col, const QString& source )
{
    // Nur tatsächlich in der Source am Ort sichtbare Symbole treffen, also nicht d_substituted
    return !sub->tok().d_substituted && sub->tok().d_lineNr == line &&
            sub->tok().d_colNr <= col && col <= ( sub->tok().d_colNr + sub->tok().d_len )
            && sub->tok().d_sourcePath == source;
}


bool CrossRefModel::findSymbolBySourcePosImp(CrossRefModel::TreePath& path, quint32 line, quint16 col,
                                             bool onlyIdents, bool hitEmpty )
{
    Q_ASSERT( !path.isEmpty() );

    // TODO: z.B. Binary search anwenden, da ja Idents alle nach Zeilen und Spalten geordnet sind
    foreach( const SymRef& sub, path.front()->children() )
    {
        path.push_front(sub);

        if( ( sub->d_tok.d_type == Tok_identifier || !onlyIdents ) &&
                isHit( sub, line, col, path.last()->d_tok.d_sourcePath ) )
            return true;

        if( findSymbolBySourcePosImp( path, line, col, onlyIdents, hitEmpty ) )
            return true;

        if( hitEmpty && hitsArea( sub, line, col, path.last()->d_tok.d_sourcePath ) )
            return true;

        //else
        path.pop_front();
    }
    return false;
}

void CrossRefModel::runUpdater(CrossRefModel* mdl)
{
    QStringList files;
    mdl->d_lock.lockForWrite();
    if( !mdl->d_work.isEmpty() )
    {
        files = mdl->d_work;
        mdl->d_work.clear();
    }
    mdl->d_lock.unlock();

    ScopeRefList scopes;
    IfDefOutLists idols;
    SectionLists secs;

    Errors errs(0,true);
    errs.setShowWarnings(false);
    errs.setReportToConsole(false);
    errs.setRecord(true);

    if( mdl->d_break )
        return;
    parseFiles( files, scopes, idols, secs, &errs, mdl->d_syms, mdl->d_incs, mdl->d_fcache, &mdl->d_break );
    if( mdl->d_break )
        return;
    mdl->insertFiles( files, scopes, idols, secs, &errs );

//    qDebug() << files.size() << "updated" << mdl->d_global.d_names.size() << "global names"
//             << mdl->d_global.d_children.size() << "global children"
//             << mdl->d_index.size() << "index entries" << mdl->d_revIndex.size() << "revindex entries";
    // TEST
    /*
    IdentDeclRef d = findDeclarationOfSymbolAtSourcePos( files.first(), 169, 6 );
    if( d.data() )
        qDebug() << "Declaration" << d->tok().d_val << d->tok().d_lineNr << d->tok().d_colNr;
        */
}

void CrossRefModel::dump(const CrossRefModel::Symbol* node, int level, bool recursive )
{
    QByteArray str;
    if( node->d_tok.d_type == Tok_Invalid )
        level--;
    else if( node->d_tok.d_type < SynTree::R_First )
    {
        if( tokenIsReservedWord( node->d_tok.d_type ) )
            str = node->d_tok.d_val.toUpper();
        else if( node->d_tok.d_type >= Tok_string )
            str = SynTree::rToStr( node->d_tok.d_type ) + QByteArray(" ") +
                    QByteArray("\"") + node->d_tok.d_val + QByteArray("\"");
        else
            str = QByteArray("\"") + node->d_tok.getName() + QByteArray("\"");

    }else
        str = SynTree::rToStr( node->d_tok.d_type );
    str += QByteArray("\t\t\t\t\t") + QFileInfo(node->d_tok.d_sourcePath).fileName().toUtf8() +
            ":" + QByteArray::number(node->d_tok.d_lineNr);
    qDebug() << QByteArray(level*3, ' ').data() << str.data();
    if( recursive )
        foreach( const SymRef& sub, node->children() )
            dump( sub.constData(), level + 1 );
}

void CrossRefModel::onWorkFinished()
{
    d_lock.lockForRead();
    bool runAgain = !d_work.isEmpty();
    d_lock.unlock();
    if( runAgain )
        d_worker->start();
}

CrossRefModel::ScopeRefNc CrossRefModel::createAst(const SynTree* top, Errors* err)
{
    ScopeRefNc res( new Scope() );
    res->d_tok = top->d_tok;
    SynTreePath synTree;
    synTree.push_front(top);
    fillAst( res.data(), res.data(), synTree, err );
    return res;
}

static bool hasScope(const SynTree* st)
{
    // Laut Std sind Scopes: module_declaration, (udp_declaration)
    //   task_declaration, function_declaration, Named blocks (begin:ident), generate_block
    switch( st->d_tok.d_type )
    {
    case SynTree::R_module_declaration:
    case SynTree::R_udp_declaration:
    case SynTree::R_task_declaration:
    case SynTree::R_function_declaration:
    case SynTree::R_list_of_ports:
    case SynTree::R_specify_block:
        return true;
    case SynTree::R_generate_block:
    case SynTree::R_seq_block:
    case SynTree::R_par_block:
#ifdef VL_O5_ONLY
        if( st->d_children.size() > 2 && st->d_children[1]->d_tok.d_type == Tok_Colon &&
                st->d_children[2]->d_tok.d_type == Tok_Identifier )
            return true;
        else
            return false;
#else
        // in SV these are scopes in any case regardless of named or unnamed
        return true;
#endif
    default:
        return false;
    }
}

static bool isDecl( quint16 type )
{
    switch( type )
    {
    case SynTree::R_library_declaration: // TODO
    case SynTree::R_config_declaration: // TODO
    case SynTree::R_udp_output_declaration:
    case SynTree::R_udp_input_declaration:
    case SynTree::R_udp_reg_declaration:
    case SynTree::R_event_declaration:
    case SynTree::R_tf_input_declaration:
    case SynTree::R_tf_output_declaration:
    case SynTree::R_tf_inout_declaration:
    case SynTree::R_genvar_declaration:
    case SynTree::R_local_parameter_declaration:
    case SynTree::R_parameter_declaration:
    case SynTree::R_port:
    case SynTree::R_integer_declaration:
    case SynTree::R_time_declaration:
    case SynTree::R_reg_declaration:
    case SynTree::R_block_integer_declaration:
    case SynTree::R_block_time_declaration:
    case SynTree::R_block_reg_declaration:
    case SynTree::R_real_declaration:
    case SynTree::R_realtime_declaration:
    case SynTree::R_block_real_declaration:
    case SynTree::R_block_realtime_declaration:
    case SynTree::R_net_declaration:
    case SynTree::R_specparam_declaration:
    case SynTree::R_module_or_udp_instantiation:
    case SynTree::R_module_or_udp_instance:
    case SynTree::R_inout_declaration:
    case SynTree::R_input_declaration:
    case SynTree::R_output_declaration:
        return true;
    default:
        return false;
    }
}

static inline bool isHierarchy( quint16 type )
{
    return type == SynTree::R_hierarchical_identifier ||
            type == SynTree::R_hierarchical_identifier_range ||
            type == SynTree::R_hierarchical_identifier_range_const;
}

static inline bool isBlockStructure( quint16 type )
{
    switch( type )
    {
    case SynTree::R_generate_block:
    case SynTree::R_seq_block:
    case SynTree::R_par_block:
        // Hier kommen wir nicht hin wenn die drei bereits in isScope abgeholt werden
        return true;
    case SynTree::R_always_construct:
    case SynTree::R_if_generate_construct:
    case SynTree::R_case_generate_construct:
    case SynTree::R_generate_region:
    case SynTree::R_loop_generate_construct:
    case SynTree::R_conditional_statement:
    case SynTree::R_case_statement:
    case SynTree::R_loop_statement:
    case Tok_Attribute:
    case Tok_MacroUsage:
        return true;
    default:
        return false;
    }
}

void CrossRefModel::fillAst(Branch* parentAst, Scope* superScope, SynTreePath& synPath , Errors* err)
{
    Q_ASSERT( parentAst != 0 && superScope != 0 && !synPath.isEmpty() );

    //int curUnnamed = 0;
    foreach( const SynTree* child, synPath.front()->d_children )
    {
//        if( child->d_tok.d_val == "yellowCounter" )
//        {
//            qDebug() << "synPath";
//            foreach( const SynTree* tmp, synPath )
//                qDebug() << SynTree::rToStr(tmp->d_tok.d_type) << tmp->d_tok.d_val;
//            qDebug() << "child" << SynTree::rToStr(child->d_tok.d_type) << child->d_tok.d_val;
//            qDebug() << "parentAst" << SynTree::rToStr(parentAst->d_tok.d_type) << parentAst->d_tok.d_val;
//            qDebug() << "superScope" << SynTree::rToStr(superScope->d_tok.d_type) << superScope->d_tok.d_val;
//        }

        if( hasScope(child) )
        {
            Scope* curScope = new Scope();
            curScope->d_super = superScope;
            curScope->d_tok = child->d_tok;
#ifndef VL_O5_ONLY
            if( curScope->d_tok.d_val.isEmpty() &&
                    ( curScope->d_tok.d_type == SynTree::R_generate_block ||
                      curScope->d_tok.d_type == SynTree::R_seq_block ||
                      curScope->d_tok.d_type == SynTree::R_par_block ))
            {
                // curScope->d_tok.d_val = QString("<block%1>").arg(curUnnamed++,2,10,QChar('0')).toUtf8();
                //static int counter = 0;
                //curScope->d_tok.d_val = QString("<%1>").arg(counter++,4,16,QChar('0')).toUtf8();
                curScope->d_tok.d_val = ".";
            }
#endif
            curScope->d_tok.d_len = calcKeyWordLen(child);
            parentAst->d_children.append( SymRef(curScope) );

            synPath.push_front(child);
            fillAst( curScope, curScope, synPath, err );
            synPath.pop_front();
        }else if( isDecl(child->d_tok.d_type) || isHierarchy(child->d_tok.d_type) ||
                  isBlockStructure(child->d_tok.d_type) )
        {
            Branch* curParent = new Branch();
            curParent->d_tok = child->d_tok;
            if( isBlockStructure(child->d_tok.d_type) )
                curParent->d_tok.d_len = calcKeyWordLen(child);
            else
                curParent->d_super = parentAst;
            parentAst->d_children.append( SymRef(curParent) );

            synPath.push_front(child);
            fillAst( curParent, superScope, synPath, err );
            synPath.pop_front();
        }else if( child->d_tok.d_type == SynTree::R_number )
        {
            checkNumber( child, err );
        }else if( child->d_tok.d_type > SynTree::R_First )
        {
            // Gehe trotzdem runter da dort noch Idents sein können
            synPath.push_front(child);
            fillAst( parentAst, superScope, synPath, err );
            synPath.pop_front();
        }else if( tokenIsBlockEnd(child->d_tok.d_type) )
        {
            // we need at least the last symbol of the production to be able to do
            // line/col based searches when the cursor is in white space
            parentAst->d_children.append( SymRef( new Symbol(child->d_tok) ) );
        }else if( child->d_tok.d_type == Tok_Rpar )
        {
            // we need trailing ) to be able to assign whitespace to production
            if( parentAst->d_tok.d_type == SynTree::R_module_or_udp_instance )
                parentAst->d_children.append( SymRef( new Symbol(child->d_tok) ) );
        }else if( child->d_tok.d_type == Tok_identifier )
        {
            Scope* scope = superScope;
            SymRefNc sym;

            if( hasScope( synPath.front() ) )
            {
                // hier kommen nur Idents an, die sich unmittelbar unter der Scope Production befinden, also
                // z.B. nicht unter port etc. isDecl und hasScope sind eindeutig unterscheidbar
                sym = new IdentDecl();
                scope = const_cast<Scope*>( superScope->d_super->toScope() );
                //qDebug() << SynTree::rToStr(superScope->d_tok.d_type) << child->d_tok.d_val;
                superScope->d_tok.d_val = child->d_tok.d_val; // damit R_module_declaration und R_udp_declaration gleich ihren Namen kennen
                superScope->d_tok.d_lineNr = child->d_tok.d_lineNr;
                superScope->d_tok.d_colNr = child->d_tok.d_colNr;
            }else if( isHierarchy(synPath.front()->d_tok.d_type ) )
            {
                sym = new PathIdent();
            }else
            {
                switch( synPath.front()->d_tok.d_type )
                {
                case SynTree::R_port:
                    // Fall port = . port_identifier
                    // Bei Ports der Art .x(y) ist y ein Symbol innerhalb des Moduls und x ist der Name des Ports
                    sym = new IdentDecl();
                    break;
                case SynTree::R_port_reference:
                    Q_ASSERT( synPath.size() > 2 // port - port_expression - port_reference
                              && synPath[1]->d_tok.d_type == SynTree::R_port_expression );
                    if( synPath[1] == synPath[2]->d_children.first() )
                    {
                        if( synPath[1]->d_children.first() == synPath.front() )
                            // Fall port = port_reference
                            sym = new IdentDecl();
                        // else
                            // Fall port = { port_reference { , port_reference } }
                            // TODO: stimmt das?
                    }else
                        // Fall port = . port_identifier ( port_expression )
                        sym = new IdentUse(); // port_expression sind Symbole innerhalb des Moduls und dort deklariert
                    break;
                case SynTree::R_module_or_udp_instantiation:
                    // Name des Moduls
                    sym = new CellRef();
                    parentAst->d_tok.d_val = child->d_tok.d_val; // damit der Branch gleich den Namen des refed Cell kennt
                    break;
                case SynTree::R_module_or_udp_instance:
                    // Name einer Modul- oder UDP-Instanz
                    sym = new IdentDecl();
                    parentAst->d_tok.d_val = child->d_tok.d_val; // damit die Instance gleich ihren Namen kennt
                    break;
                case SynTree::R_name_of_gate_instance:
                    // Name einer gate_instantiation
                    sym = new IdentDecl();
                    break;
                case SynTree::R_port_connection_or_output_terminal:
                case SynTree::R_named_parameter_assignment:
                    sym = new PortRef();
                    break;
                case SynTree::R_udp_output_declaration:
                case SynTree::R_udp_input_declaration:
                case SynTree::R_udp_reg_declaration:
                case SynTree::R_list_of_event_identifiers: // event_declaration
                case SynTree::R_tf_input_declaration:
                case SynTree::R_tf_output_declaration:
                case SynTree::R_tf_inout_declaration:
                case SynTree::R_param_assignment: // local_parameter_declaration, parameter_declaration
                case SynTree::R_genvar_declaration:
                case SynTree::R_variable_type:  // integer_declaration, time_declaration, reg_declaration
                case SynTree::R_real_type: // real_declaration, realtime_declaration
                case SynTree::R_block_variable_type: // block_integer_declaration, block_time_declaration, block_reg_declaration
                case SynTree::R_block_real_type: // block_real_declaration, block_realtime_declaration
                case SynTree::R_list_of_net_decl_assignments_or_identifiers: // net_declaration
                case SynTree::R_specparam_assignment:   // specparam_declaration
                    sym = new IdentDecl();
                    break;
                    // TODO: pulse_control_specparam ?

                    // inout, input und output dürfen entweder in list_of_port_declarations oder direkt
                    // in module_declaration, falls list_of_ports vorhanden ist, erscheinen; ansonsten Fehler
                    // list_of_port_declarations | module_declaration - inout_declaration | input_declaration | output_declaration
                case SynTree::R_inout_declaration:
                case SynTree::R_input_declaration:
                case SynTree::R_output_declaration:
                    Q_ASSERT( synPath.size() > 1 );
                    if( synPath[1]->d_tok.d_type == SynTree::R_list_of_port_declarations )
                        sym = new IdentDecl();
                    else
                    {
                        Q_ASSERT( synPath[1]->d_tok.d_type == SynTree::R_module_declaration );
                        checkLop( superScope, child, err );
                        // wird zu IdenUse, der dann auf Port referenzieren kann
                    }
                    break;
                case SynTree::R_list_of_variable_port_identifiers: // output_declaration
                    // list_of_port_declarations | module_declaration - output_declaration - list_of_variable_port_identifiers
                    Q_ASSERT( synPath.size() > 2 );
                    if( synPath[2]->d_tok.d_type == SynTree::R_list_of_port_declarations )
                        sym = new IdentDecl();
                    else
                    {
                        Q_ASSERT( synPath[2]->d_tok.d_type == SynTree::R_module_declaration );
                        checkLop( superScope, child, err );
                        // wird zu IdenUse, der dann auf Port referenzieren kann
                    }
                    break;
                }
            }

            if( sym.constData() == 0 )
                sym = new IdentUse();

            sym->d_tok = child->d_tok;
            parentAst->d_children.append(sym);
            if( IdentDecl* id = const_cast<IdentDecl*>( sym->toIdentDecl() ) )
            {
                id->d_decl = parentAst;
                const IdentDecl*& slot = scope->d_names[id->d_tok.d_val];
                if( slot != 0 )
                    err->error(Errors::Semantics, id->d_tok.d_sourcePath, id->d_tok.d_lineNr,id->d_tok.d_colNr,
                                  tr("duplicate name: %1").arg(id->d_tok.d_val.data()) );
                else
                    slot = id;
            }
        }
    }
}

void CrossRefModel::checkNumber(const SynTree* number, Errors* err)
{
    QByteArray str;
    foreach( const SynTree* child, number->d_children )
    {
        if( !child->d_tok.d_prePp )
            str += child->d_tok.d_val;
    }

    Vl::NumLex lex(str);
    if( !lex.parse(true) )
    {
        err->error(Errors::Syntax, number->d_tok.d_sourcePath, number->d_tok.d_lineNr,number->d_tok.d_colNr,
                      tr("number %1: %2").arg(str.data()).arg(lex.getError()) );
    }
}

bool CrossRefModel::checkLop(CrossRefModel::Scope* superScope, const SynTree* id, Errors* err)
{
    const Scope* lop = Symbol::toScope( findFirst(superScope, SynTree::R_list_of_ports) );
    if( lop == 0 )
    {
        err->error(Errors::Semantics, id->d_tok.d_sourcePath, id->d_tok.d_lineNr,id->d_tok.d_colNr,
                      tr("port_declaration not allowed here if list_of_ports declaration is not used") );
        return false;
    }
    if( !lop->d_names.contains( id->d_tok.d_val ) )
    {
        err->error(Errors::Semantics, id->d_tok.d_sourcePath, id->d_tok.d_lineNr,id->d_tok.d_colNr,
                      tr("port_declaration must correspond to one in the list_of_ports: %1").arg(id->d_tok.d_val.data()) );
        return false;
    }else if( lop->d_names.contains( char(1) + id->d_tok.d_val ) )
    {
        err->error(Errors::Semantics, id->d_tok.d_sourcePath, id->d_tok.d_lineNr,id->d_tok.d_colNr,
                      tr("duplicate port_declaration: %1").arg(id->d_tok.d_val.data()) );
        return false;
    }
    return true;
}

CrossRefModel::Symbol::Symbol(const Token& t):d_tok(t)
{

}

const CrossRefModel::Symbol::SymRefList& CrossRefModel::Symbol::children() const
{
    static SymRefList dummy;
    return dummy;
}

const CrossRefModel::Branch*CrossRefModel::Symbol::toBranch() const
{
    // more efficient than dynamic_cast or typeid, see ISO/IEC TR 18015:2004 5.3.8
    switch( getType() )
    {
    case ClassScope:
    case ClassBranch:
        return static_cast<const Branch*>(this);
    default:
        return 0;
    }
}

const CrossRefModel::Scope*CrossRefModel::Symbol::toScope() const
{
    if( getType() == ClassScope )
        return static_cast<const Scope*>(this);
    else
        return 0;
}

const CrossRefModel::Scope*CrossRefModel::Symbol::toScope(const CrossRefModel::Symbol* sym)
{
    if( sym == 0 )
        return 0;
    else
        return sym->toScope();
}

const CrossRefModel::IdentDecl*CrossRefModel::Symbol::toIdentDecl() const
{
    if( getType() == ClassIdentDecl )
        return static_cast<const IdentDecl*>(this);
    else
        return 0;
}

const CrossRefModel::CellRef*CrossRefModel::Symbol::toCellRef() const
{
    if( getType() == ClassCellRef )
        return static_cast<const CellRef*>(this);
    else
        return 0;
}

const CrossRefModel::IdentUse*CrossRefModel::Symbol::toIdentUse() const
{
    if( getType() == ClassIdentUse )
        return static_cast<const IdentUse*>(this);
    else
        return 0;
}

const CrossRefModel::PathIdent*CrossRefModel::Symbol::toPathIdent() const
{
    if( getType() == ClassPathIdent )
        return static_cast<const PathIdent*>(this);
    else
        return 0;
}

const CrossRefModel::PortRef*CrossRefModel::Symbol::toPortRef() const
{
    if( getType() == ClassPortRef )
        return static_cast<const PortRef*>(this);
    else
        return 0;
}

const char*CrossRefModel::Symbol::getTypeName() const
{
    switch( getType() )
    {
    case ClassSymbol:
        return "Symbol";
    case ClassBranch:
        return "Branch";
    case ClassIdentDecl:
        return "IdentDecl";
    case ClassIdentUse:
        return "IdentUse";
    case ClassPathIdent:
        return "PathIdent";
    case ClassCellRef:
        return "CellRef";
    case ClassPortRef:
        return "PortRef";
    case ClassScope:
        return "Scope";
    default:
        return "Unknown Type";
    }
}

const CrossRefModel::SymRefList& CrossRefModel::Branch::children() const
{
    return d_children;
}

const CrossRefModel::Symbol* CrossRefModel::findFirst(const Branch* b, quint16 type)
{
    foreach( const SymRef& sub, b->d_children )
    {
        if( sub && sub->d_tok.d_type == type )
            return sub.data();
    }
    return 0;
}

#ifdef _DUMP_AST
#include <typeinfo>
#include <cxxabi.h>
static void dumpAst(const CrossRefModel::Symbol* l, int level)
{
    int status;
    char * demangled = abi::__cxa_demangle(typeid(*l).name(),0,0,&status);
    qDebug() << QByteArray(level*3,' ').data() << SynTree::rToStr(l->tok().d_type) << l->tok().d_lineNr
             << l->tok().d_colNr << l->tok().d_len << l->tok().d_val << "\t\t" << demangled+19;
    free(demangled);
    foreach( const CrossRefModel::SymRef& sub, l->children() )
        dumpAst( sub.constData(), level+1 );
}
#endif

int CrossRefModel::parseFiles(const QStringList& files, ScopeRefList& scopes, IfDefOutLists& idols, SectionLists& secs,
                               Errors* errs, PpSymbols* syms, Includes* incs, FileCache* fcache, QAtomicInt* stop )
{
    QElapsedTimer t;
    t.start();
    int esum = 0;
    foreach( const QString& file, files )
    {
        if( stop && *stop )
            return esum;
        IfDefOutLists idol;
        SectionList sec;
        int e = errs->getErrCount();
        int w = errs->getWrnCount();
        parseStream( 0, file, scopes, idol, sec, errs, syms, incs, fcache );
        secs[file] = sec;
        IfDefOutLists::const_iterator i;
        for( i = idol.begin(); i != idol.end(); ++i )
            idols.insert( i.key(), i.value() );
        e = errs->getErrCount() - e;
        w = errs->getWrnCount() - w;
//        if( errs->reportToConsole() )
//        {
//            if( e != 0 && w != 0 )
//                qDebug() << "####" << e << "Errors and" << w << "Warnings in" << file;
//            else if( errs->showWarnings() && w != 0 )
//                qDebug() << "####" << w << "Warnings in" << file;
//            else if( e != 0)
//                qDebug() << "####" << e << "Errors in" << file;
//        }
        esum += e;
    }
//    if( errs->reportToConsole() )
//        qDebug() << "### Parsed" << files.size() << "files in" << t.elapsed() << "ms with" << esum << "errors";
    return esum;
}

bool CrossRefModel::parseStream(QIODevice* stream, const QString& sourcePath, CrossRefModel::ScopeRefList& refs,
                                CrossRefModel::IfDefOutLists& idols, SectionList& secs, Errors* errs, PpSymbols* syms,
                                Includes* incs, FileCache* fcache)
{
    PpLexer lex;
    lex.setErrors( errs );
    lex.setSyms( syms );
    lex.setIncs( incs );
    lex.setCache(fcache);
    lex.setIgnoreAttrs(false);
    lex.setPackAttrs(false);
    lex.setSendMacroUsage(true);

    lex.setStream( stream, sourcePath );

    Parser p;
    const bool res = p.parseFile( &lex, errs );
    if( true ) // res ) // we need a SynTree in any case even with syntax errors
    {
        SynTree root;
        root.d_children = p.getResult(true);
        ScopeRef top = createAst( &root, errs );
//        qDebug() << "*** parsed" << sourcePath;
//        foreach( const SymRef& sym, top->children() )
//            qDebug() << sym->d_tok.d_val << sym->d_tok.d_lineNr;
        refs.append(top);
        idols = lex.getIdols();
        QList<int> stack;
        foreach( Token tok, p.getSections() )
        {
            if( tok.d_sourcePath != sourcePath )
                continue;
            if( tok.d_type == Tok_Section )
            {
                Section s;
                s.d_lineFrom = s.d_lineTo = tok.d_lineNr;
                s.d_title = tok.d_val;
                secs.append(s);
                stack.push_back( secs.size() - 1 );
            }else if( tok.d_type == Tok_SectionEnd )
            {
                if( !stack.isEmpty() )
                {
                    int i = stack.back();
                    stack.pop_back();
                    Q_ASSERT( i < secs.size() );
                    secs[i].d_lineTo = tok.d_lineNr;
                }
            }
        }

#ifdef _DUMP_AST
        qDebug() << "********** dumping" << file;
        dumpAst(top.constData(), 0 );
        qDebug() << "********** end dump" << file;
#endif
//        qDebug() << "************** name table of" << file;
//        Scope::Names::const_iterator n;
//        for( n = top->d_names.begin(); n != top->d_names.end(); ++n )
//            qDebug() << n.key() << SynTree::rToStr(n.value()->d_tok.d_type) << n.value()->d_tok.d_lineNr;
    }
    return res;
}

void CrossRefModel::insertFiles(const QStringList& files, const ScopeRefList& scopes,
                                const IfDefOutLists& idols, const SectionLists& secs, Errors* errs, bool lock )
{
    if( lock )
        d_lock.lockForRead();
    Scope newGlobal = d_global;
    IfDefOutLists newIdols = d_idols;
    SectionLists newSecs = d_sections;
    if( lock )
        d_lock.unlock();

    QElapsedTimer t;
    t.start();

    int errCount = errs->getErrCount();

    // Lösche zuerst alles, was die neu geparsten Files betrifft, aus dem existierenden Global
    foreach( const QString& file, files )
        clearFile(&newGlobal,file);

    // scopes enthält für jedes geparste File einen Scope
    foreach( const ScopeRef& scope, scopes )
    {
        // Prüfe, ob die Namen in scope allenfalls schon existieren, ansonsten füge sie in Global
        Scope::Names::const_iterator scopeIter;
        for( scopeIter = scope->d_names.begin(); scopeIter != scope->d_names.end(); ++scopeIter )
        {
            Scope::Names::const_iterator globalIter = newGlobal.d_names.find( scopeIter.key() );
            if( globalIter != newGlobal.d_names.end() )
            {
                const IdentDecl* newDecl = scopeIter.value();
                const IdentDecl* existingDecl = globalIter.value();
                errs->error(Errors::Semantics, newDecl->d_decl->d_tok.d_sourcePath,
                            newDecl->d_decl->d_tok.d_lineNr, newDecl->d_decl->d_tok.d_colNr,
                              tr("duplicate cell name '%1' already declared in %2")
                            .arg( newDecl->d_tok.d_val.data() )
                            .arg( existingDecl->d_tok.d_sourcePath ) );
            }else
                newGlobal.d_names.insert( scopeIter.key(), scopeIter.value() );

        }
        // Alle children von scope werden übernommen und auf den neuen Global angepasst
        foreach( const SymRef& s, scope->children() )
        {
            Branch* b = const_cast<Branch*>( s->toBranch() );
            b->d_super = &newGlobal;
        }
        newGlobal.d_children += scope->d_children;

    }

    Index index;
    RevIndex revIndex;
    foreach( const SymRef& sub, newGlobal.d_children )
    {
        const Scope* cell = sub->toScope();
        // NOTE: the d_super of all scopes not in files still point to d_global! We don't care since this is
        // the only thread causing mutations and the mutations are serialized
        if( cell ) // sub may be 0!
            resolveIdents( index, revIndex, cell, 0, cell, &newGlobal, errs );
    }

    IfDefOutLists::const_iterator i;
    for( i = idols.begin(); i != idols.end(); ++i )
        newIdols.insert( i.key(), i.value() );

    SectionLists::const_iterator j;
    for( j = secs.begin(); j != secs.end(); ++j )
        newSecs.insert( j.key(), j.value() );

    errCount = errs->getErrCount() - errCount;

//    if( errs->reportToConsole() )
//        qDebug() << "### Elaborated and indexed" << files.size() << "files in" << t.elapsed()
//                 << "ms with" << errCount << "errors";

    if( lock )
        d_lock.lockForWrite();
    t.restart();
    d_index = index;
    d_revIndex = revIndex;
    d_idols = newIdols;
    d_sections = newSecs;
    d_global.d_names = newGlobal.d_names;
    d_global.d_children = newGlobal.d_children;
    foreach( const SymRef& sub, d_global.d_children )
    {
        Scope* cell = const_cast<Scope*>( sub->toScope());
        if( cell ) // sub may be 0
            cell->d_super = &d_global;
    }
    d_errs->clearFiles(files);
    d_errs->update( *errs );
//    if( errs->reportToConsole() )
//        qDebug() << "### Replaced global scope in" << t.elapsed() << "ms";
    if( lock )
        d_lock.unlock();

    emit sigModelUpdated();
    foreach( const QString& file, files )
        emit sigFileUpdated(file);
}

void CrossRefModel::clearFile(Scope* global, const QString& file)
{
    SymRefList children = global->d_children;
    for( int i = 0; i < children.size(); i++ )
    {
        if( children[i]->d_tok.d_sourcePath == file )
        {
            Q_ASSERT( !children[i]->d_tok.d_val.isEmpty() );
            global->d_names.remove( children[i]->d_tok.d_val );
            children[i] = 0; // statt löschen einfach auf Null setzen, geht schneller
        }
    }
    global->d_children.clear();
    for( int i = 0; i < children.size(); i++ )
    {
        if( children[i].constData() != 0 )
            global->d_children.append(children[i]);
    }
}

bool CrossRefModel::insertCell(const IdentDecl* decl, Scope* global, Errors* errs)
{
    Q_ASSERT( false ); // obsolete funktion, direkt in caller eingefügt

    // Ein erheblicher Teil der Module (n=1173) unterscheidet sich im Namen vom Dateinamen
    // Häufig nur in Gross-Kleinschreibung, häufig mit "_postfix"
    // Der Versuch kann also nicht bestätigen, dass mehrheitlich ein Modul pro Datei und gleich heisst.
//    if( decl->tok().d_val != QFileInfo(decl->tok().d_sourcePath).baseName() )
//        qDebug() << "**** Module name differs from file name" << decl->tok().d_val << decl->tok().d_sourcePath;
    Scope* scope = const_cast<Scope*>( decl->d_decl->toScope() );
    Q_ASSERT( scope != 0 );
    const int pos = global->d_children.indexOf( SymRef(scope) );
    if( pos == -1 )
        global->d_children.append( SymRef(scope) );
    else
        global->d_children[pos] = scope;
    scope->d_super = global;
    Scope::Names::const_iterator i = global->d_names.find( decl->d_tok.d_val );
    if( i != global->d_names.end() )
    {
        const IdentDecl* d = i.value();
        errs->error(Errors::Semantics, scope->d_tok.d_sourcePath, scope->d_tok.d_lineNr, scope->d_tok.d_colNr,
                      tr("duplicate cell name '%1' already declared in %2")
                    .arg(scope->d_tok.d_val.data())
                    .arg( d->tok().d_sourcePath ) );
        return false;
    }else
    {
        global->d_names.insert( decl->d_tok.d_val, decl );
        return true;
    }
}

void CrossRefModel::resolveIdents(Index& index, RevIndex& revIndex, const Symbol* leaf, const Branch* parent,
                                  const Scope* curScope, const Scope* globScope, Errors* errs)
{
    Q_ASSERT( leaf != 0 );
    if( const IdentUse* use = leaf->toIdentUse() )
    {
        const IdentDecl* id = findNameInScope( curScope, use->d_tok.d_val, true, true );
        if( id != 0 )
        {
            index.insert( use, id );
            revIndex.insert( id, use );
        }else if( parent->tok().d_type != Tok_Attribute )
            errs->error(Errors::Semantics, use->d_tok.d_sourcePath, use->d_tok.d_lineNr, use->d_tok.d_colNr,
                          tr("unknown identifier: %1").arg(use->d_tok.d_val.data()) );
    }else if( const PathIdent* use = leaf->toPathIdent() )
    {
        // Jeder hierarchical_identifier/_x enthält als children die ganze Kette von Idents
        if( use == parent->d_children.first().data() )
        {
            // Wir lösen die Kette nur beim ersten Ident auf
            QList<const PathIdent*> path;
            path << use;
            for(int i = 1; i < parent->d_children.size(); i++ )
            {
                const PathIdent* p = parent->d_children[i]->toPathIdent();
                if( p )
                    path << p;
            }
            const Scope* scope = curScope;
            for( int i = 0; i < path.size(); i++ )
            {
                //if( path[i]->d_tok.d_val == "s" )
                //    qDebug() << "hit" << path[i]->d_tok.d_lineNr << path[i]->d_tok.d_colNr;

                const IdentDecl* id = findNameInScope( scope, path[i]->d_tok.d_val, true, true );
                if( id != 0 )
                {
                    index.insert( path[i], id );
                    revIndex.insert( id, path[i] );
                    if( i < path.size() - 1 )
                    {
                        scope = 0;
                        Q_ASSERT( id->d_decl != 0 );
                        if( id->d_decl->d_tok.d_type == SynTree::R_module_or_udp_instance )
                        {
                            Q_ASSERT( id->d_decl->d_super != 0 && id->d_decl->d_super->d_tok.d_type == SynTree::R_module_or_udp_instantiation );

                            const IdentDecl* cellId = findNameInScope( globScope, id->d_decl->d_super->d_tok.d_val, false, false );
                            if( cellId != 0 )
                                scope = cellId->decl()->toScope();
                        }else
                            scope = id->d_decl->toScope();
                        if( scope == 0 )
                        {
                            errs->error(Errors::Semantics, id->d_tok.d_sourcePath, id->d_tok.d_lineNr, id->d_tok.d_colNr,
                                          tr("identifier is not a name space: %1").arg(id->d_tok.d_val.data()) );
                            break;
                        }
                    }
                }else
                    errs->error(Errors::Semantics, path[i]->d_tok.d_sourcePath, path[i]->d_tok.d_lineNr, path[i]->d_tok.d_colNr,
                                  tr("unknown identifier: %1").arg(path[i]->d_tok.d_val.data()) );
            }
        }
    }else if( const PortRef* ref = leaf->toPortRef() )
    {
        Q_ASSERT( parent != 0 && parent->d_super != 0 );
        QByteArray moduleName;
        if( parent->d_tok.d_type == SynTree::R_module_or_udp_instance )
        {
            // Kommt bei Parametern weiterhin vor
            Q_ASSERT( parent->d_super->d_tok.d_type == SynTree::R_module_or_udp_instantiation );
            moduleName = parent->d_super->d_tok.d_val;
        }else
        {
            Q_ASSERT( parent->d_tok.d_type == SynTree::R_module_or_udp_instantiation );
            moduleName = parent->d_tok.d_val;
        }
        const IdentDecl* cellId = findNameInScope( globScope, moduleName, false, false );
        if( cellId != 0 )
        {
            const Scope* cell = cellId->decl()->toScope();
            Q_ASSERT( cell != 0 );
            const IdentDecl* id = findNameInScope( cell, ref->d_tok.d_val, false, true );
            if( id != 0 )
            {
                index.insert( ref, id );
                revIndex.insert( id, ref );
            }else
                errs->error(Errors::Semantics, ref->d_tok.d_sourcePath, ref->d_tok.d_lineNr, ref->d_tok.d_colNr,
                              tr("unknown port or parameter: %1").arg(ref->d_tok.d_val.data()) );
        }
    }else if( const CellRef* ref = leaf->toCellRef() )
    {
        const IdentDecl* cellId = findNameInScope( globScope, ref->d_tok.d_val, false, false );
        if( cellId != 0 )
        {
            index.insert( ref, cellId );
            revIndex.insert( cellId, ref );
        }else
            errs->error(Errors::Elaboration, ref->d_tok.d_sourcePath, ref->d_tok.d_lineNr, ref->d_tok.d_colNr,
                          tr("unknown module or udp: %1").arg(ref->d_tok.d_val.data()) );
    }
    foreach( const SymRef& sub, leaf->children() )
    {
        const Scope* scope = sub->toScope();
        if( scope == 0 )
            scope = curScope;
        resolveIdents(index, revIndex, sub.data(), leaf->toBranch(), scope, globScope, errs );
    }
}

CrossRefModel::Branch::Branch():d_super(0)
{

}

CrossRefModel::Scope::Names2 CrossRefModel::Scope::getNames2(bool recursive) const
{
    Names2 res;
    const Scope* super = d_super ? d_super->toScope() : 0;
    if( recursive && super != 0 )
        res = super->getNames2(recursive);

    if( d_tok.d_type == SynTree::R_module_declaration )
    {
        const Scope* lop = Symbol::toScope( findFirst( this, SynTree::R_list_of_ports) );
        if( lop )
        {
            Names::const_iterator i;
            for( i = lop->d_names.begin(); i != lop->d_names.end(); ++i )
                res.insert( i.key(), IdentDeclRef(i.value()) );
        }
    }

    Names::const_iterator i;
    for( i = d_names.begin(); i != d_names.end(); ++i )
        res.insert( i.key(), IdentDeclRef(i.value()) );

    return res;
}

CrossRefModel::IdentDeclRefList CrossRefModel::Scope::getNames() const
{
    IdentDeclRefList res;
    if( d_tok.d_type == SynTree::R_module_declaration )
    {
        const Scope* lop = Symbol::toScope( findFirst( this, SynTree::R_list_of_ports) );
        if( lop )
        {
            Names::const_iterator i;
            for( i = lop->d_names.begin(); i != lop->d_names.end(); ++i )
                res.append( IdentDeclRef(i.value()) );
        }
    }
    Names::const_iterator i;
    for( i = d_names.begin(); i != d_names.end(); ++i )
        res.append( IdentDeclRef(i.value()) );
    return res;
}
