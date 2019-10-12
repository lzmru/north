#ifndef LIBNORTH_AST_DECLARATIONS_INTERFACEDECL_H
#define LIBNORTH_AST_DECLARATIONS_INTERFACEDECL_H

namespace north::ast {

class InterfaceDecl : public GenericDecl {
  using RequiredList = std::vector<FunctionDecl *>;
  
  RequiredList Required;
  InterfaceDecl *Parent;

public:
  explicit InterfaceDecl(const TokenInfo &TkInfo)
      : GenericDecl(TkInfo.Pos, AST_InterfaceDecl, TkInfo.toString()),
        Parent(nullptr) {}

  void addFunction(FunctionDecl *Function) { Required.push_back(Function); }
  const RequiredList &getDemands() const { return Required; }

  InterfaceDecl *getParent() { return Parent; }
  void setParent(InterfaceDecl *NewParent) { Parent = NewParent; }

  AST_NODE(InterfaceDecl)
};

} // namespace north::ast

#endif // LIBNORTH_AST_DECLARATIONS_INTERFACEDECL_H
