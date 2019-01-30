#ifndef VLCROSSREFMODEL_H
#define VLCROSSREFMODEL_H

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

#include <QObject>
#include <QSharedData>
#include <QMap>
#include <QReadWriteLock>
#include <QStringList>
#include <Verilog/VlToken.h>

namespace Vl
{
    class SynTree;
    class Errors;
    class PpSymbols;
    class Includes;
    class PpLexer;
    class Parser;
    class FileCache;

    class CrossRefModel : public QObject
    {
        Q_OBJECT
        // this class is thread-safe
    public:
        typedef QList<quint32> IfDefOutList; // Jeder Eintrag ist die Zeile der Änderung. Start bei On.

        struct Section
        {
            quint32 d_lineFrom, d_lineTo;
            QByteArray d_title;
            Section():d_lineFrom(0),d_lineTo(0){}
        };
        typedef QList<Section> SectionList;

        class Branch;
        class Scope;
        class IdentDecl;
        class CellRef;
        class IdentUse;
        class PathIdent;
        class PortRef;

        enum { ClassSymbol, ClassBranch, ClassIdentDecl, ClassIdentUse, ClassPathIdent, ClassCellRef,
               ClassPortRef, ClassScope };

        class Symbol : public QSharedData
        {
        public:
            typedef QExplicitlySharedDataPointer<const Symbol> SymRef;
            typedef QList<SymRef> SymRefList;

            Symbol(const Token& = Token() );

            const Token& tok() const { return d_tok; }
            virtual const SymRefList& children() const;
            const Branch* toBranch() const;
            const Scope* toScope() const;
            static const Scope* toScope(const Symbol*);
            const IdentDecl* toIdentDecl() const;
            const CellRef* toCellRef() const;
            const IdentUse* toIdentUse() const;
            const PathIdent* toPathIdent() const;
            const PortRef* toPortRef() const;
            virtual int getType() const { return ClassSymbol; }
            const char* getTypeName() const;
        protected:
            virtual ~Symbol() {}

            friend class QExplicitlySharedDataPointer<Symbol>;
            friend class QExplicitlySharedDataPointer<const Symbol>;

        private:
            friend class CrossRefModel;
            Token d_tok;
        };

        typedef QExplicitlySharedDataPointer<const Symbol> SymRef;
        typedef QList<SymRef> SymRefList;
        typedef SymRefList TreePath; // index 0: bottom, index max: root

        class Branch : public Symbol
        {
        public:
            Branch();
            Branch* super() const { return d_super; }
            virtual const SymRefList& children() const Q_DECL_OVERRIDE;
            // TODO: Position des End-Tokens der Deklaration zwecks Kurzdarstellung bei Hover
            int getType() const Q_DECL_OVERRIDE { return ClassBranch; }
        private:
            friend class CrossRefModel;
            Branch* d_super; // Points to enclosing scope (for Scopes) and to super Branch (for decls)
            SymRefList d_children;
        };

        class IdentDecl : public Symbol
        {
        public:
            const Branch* decl() const { return d_decl; }
            int getType() const Q_DECL_OVERRIDE { return ClassIdentDecl; }
        protected:
            IdentDecl():d_decl(0){}
        private:
            friend class CrossRefModel;
            Branch* d_decl; // points to Decl for Decl Idents, to containing AstNode for all others
        };
        typedef QExplicitlySharedDataPointer<const IdentDecl> IdentDeclRef;
        typedef QList<IdentDeclRef> IdentDeclRefList;

        class IdentUse : public Symbol
        {
        public:
            int getType() const Q_DECL_OVERRIDE { return ClassIdentUse; }
        };

        class PathIdent : public Symbol // Ident is part of hierarchical path
        {
        public:
            int getType() const Q_DECL_OVERRIDE { return ClassPathIdent; }
        };

        class CellRef : public Symbol // Name is a reference to a module or udp
        {
        public:
            int getType() const Q_DECL_OVERRIDE { return ClassCellRef; }
        };

        class PortRef : public Symbol // Name is a reference to a port or parameter name in a module or udp
        {
        public:
            int getType() const Q_DECL_OVERRIDE { return ClassPortRef; }
        };

        class Scope : public Branch
        {
        public:
            IdentDeclRefList getNames() const;
        protected:
            int getType() const Q_DECL_OVERRIDE { return ClassScope; }
        private:
            friend class CrossRefModel;
            typedef QMap<QByteArray,const IdentDecl*> Names; // TODO: Hash
            Names d_names;
        };
        typedef QExplicitlySharedDataPointer<const Scope> ScopeRef;


        explicit CrossRefModel(QObject *parent = 0, FileCache* = 0);
        ~CrossRefModel();

        bool updateFiles( const QStringList&, bool synchronous = false );
        bool parseString( const QString& code, const QString& sourcePath = QString() );
        void clear();

        PpSymbols* getSyms() const { return d_syms; }
        Includes* getIncs() const { return d_incs; }
        Errors* getErrs() const { return d_errs; }

        bool isEmpty() const;

        TreePath findSymbolBySourcePos(const QString& file, quint32 line, quint16 col , bool onlyIdents = true ) const;
        IdentDeclRef findDeclarationOfSymbolAtSourcePos(const QString& file, quint32 line, quint16 col) const;
        IdentDeclRef findDeclarationOfSymbol(const Symbol* ) const;
        SymRefList findAllReferencingSymbols(const Symbol* ) const;
        SymRefList findReferencingSymbolsByFile(const Symbol*, const QString& file ) const;
        IfDefOutList getIfDefOutsByFile( const QString& file ) const;
        SectionList getSections( const QString& file ) const;
        Section findSectionBySourcePos( const QString& file, quint32 line, quint16 col ) const;
        SymRef findGlobal( const QByteArray& name ) const;
        SymRefList getGlobalSyms( const QString& file = QString() ) const;
        IdentDeclRefList getGlobalNames( const QString& file = QString() ) const;
        static QList<Token> findTokenByPos(const QString& line, int col, int* pos );
        static QString qualifiedName( const TreePath&, bool skipFirst = false );
        static QStringList qualifiedNameParts( const TreePath&, bool skipFirst = false );

    signals:
        void sigFileUpdated( const QString& path );
        void sigModelUpdated();

    protected:
        typedef QList<const SynTree*> SynTreePath; // top = last
        typedef QMap<QString,IfDefOutList> IfDefOutLists; // file -> list
        typedef QMap<QString,SectionList> SectionLists; // file -> list
        typedef QList<ScopeRef> ScopeRefList;
        typedef QHash<const Symbol*,const IdentDecl*> Index; // ident use -> ident declaration
        typedef QMultiHash<const Symbol*, const Symbol*> RevIndex;
        typedef QExplicitlySharedDataPointer<Scope> ScopeRefNc;
        typedef QExplicitlySharedDataPointer<Symbol> SymRefNc;

        static ScopeRefNc createAst( const SynTree*, Errors* ); // returns a global scope
        static void fillAst( Branch* parentAst, Scope* superScope, SynTreePath& synPath, Errors* );
        static void checkNumber( const SynTree*, Errors* );
        static bool checkLop(Scope* superScope, const SynTree* id, Errors* );
        static const Symbol* findFirst(const Branch*, quint16 type);

        static int parseFiles(const QStringList& files, ScopeRefList&, IfDefOutLists&, SectionLists&,
                                Errors* errs, PpSymbols* syms, Includes* incs , FileCache* fcache, QAtomicInt* );
        static bool parseStream(QIODevice* stream, const QString& sourcePath, ScopeRefList&, IfDefOutLists&, SectionList&,
                              Errors* errs, PpSymbols* syms, Includes* incs , FileCache* fcache);
        void insertFiles(const QStringList& files, const ScopeRefList&, const IfDefOutLists&, const SectionLists&, Errors* errs , bool lock = true); // write lock
        static void clearFile(Scope*, const QString& file);
        static bool insertCell(const IdentDecl* decl, Scope*, Errors* );
        static void resolveIdents( Index&, RevIndex&, const Symbol*, const Branch*, const Scope*, const Scope*, Errors* );
        static const IdentDecl* findNameInScope( const Scope*, const QByteArray& name, bool recursiv = true, bool ports = false );
        static quint16 calcTextLenOfDecl( const SynTree* );
        static quint16 calcKeyWordLen( const SynTree* );
        static bool findSymbolBySourcePosImp(TreePath& path, quint32 line, quint16 col, bool onlyIdents );
        static void runUpdater(CrossRefModel* );
    protected slots:
        void onWorkFinished();
    private:
        Errors* d_errs;
        PpSymbols* d_syms;
        Includes* d_incs;
        FileCache* d_fcache;

        Scope d_global;
        IfDefOutLists d_idols;
        SectionLists d_sections;
        Index d_index;
        RevIndex d_revIndex;

        mutable QReadWriteLock d_lock;
        QStringList d_work; // no set because order may be relevant
        class Worker;
        Worker* d_worker;
        QAtomicInt d_break;
    };
}
Q_DECLARE_METATYPE(Vl::CrossRefModel::SymRef)

#endif // VLCROSSREFMODEL_H
