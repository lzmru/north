#ifndef LIBNORTH_AST_DECLARATIONS_STRUCTDECL_H
#define LIBNORTH_AST_DECLARATIONS_STRUCTDECL_H

namespace north::ast {

class StructDecl final : public GenericDecl {
  std::vector<VarDecl *> Fields;
  llvm::StructType *IR = nullptr;

public:
  explicit StructDecl(const Position &Pos, bool IsPacked = false)
      : GenericDecl(Pos, AST_StructDecl, "") {}

  llvm::ArrayRef<VarDecl *> getFieldList() { return Fields; }
  VarDecl *getField(uint8_t I) { return Fields[I]; }
  void addField(VarDecl *Field) { Fields.push_back(Field); }

  llvm::StructType *getIR() { assert(IR); return IR; }
  void setIR(llvm::StructType *NewIR) {
    assert(NewIR);
    IR = NewIR;
  }

  AST_NODE(StructDecl)
};

} // namespace north::ast

#endif // LIBNORTH_AST_DECLARATIONS_STRUCTDECL_H
