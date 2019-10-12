#ifndef LIBNORTH_AST_DECLARATIONS_STRUCTDECL_H
#define LIBNORTH_AST_DECLARATIONS_STRUCTDECL_H

namespace north::ast {

class StructDecl : public GenericDecl {
  std::vector<VarDecl *> Fields;
  llvm::StructType *IRValue = nullptr;

public:
  explicit StructDecl(const Position &Pos, bool IsPacked = false)
      : GenericDecl(Pos, AST_StructDecl, "") {}

  llvm::ArrayRef<VarDecl *> getFieldList() { return Fields; }
  VarDecl *getField(uint8_t I) { return Fields[I]; }
  void addField(VarDecl *Field) { Fields.push_back(Field); }

  llvm::StructType *getIR() { return IRValue; }
  void setIR(llvm::Type *NewIRValue) {
    IRValue = (llvm::StructType *)NewIRValue;
  }

  AST_NODE(StructDecl)
};

} // namespace north::ast

#endif // LIBNORTH_AST_DECLARATIONS_STRUCTDECL_H
