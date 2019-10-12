#ifndef LIBNORTH_AST_DECLARATIONS_GENERICDECL_H
#define LIBNORTH_AST_DECLARATIONS_GENERICDECL_H

namespace north::ast {

class GenericDecl : public Declaration {
public:
  struct Generic {
    Position Pos;
    llvm::StringRef Name;
    type::Type *Type;

    explicit Generic(Position Pos, llvm::StringRef Name)
        : Pos(Pos), Name(Name), Type(nullptr) {}
  };
  using GenericList = std::vector<Generic>;

  enum Modifier { None = 0, Ptr = 1, Ref = 2, Out = 4, In = 8 };

private:
  uint8_t Modifiers;
  GenericList Generics;

public:
  GenericDecl(const Position &Pos, NodeKind Kind, llvm::StringRef Identifier, Modifier Mods = None)
      : Declaration(Pos, Kind, Identifier), Modifiers(Mods) {}

  bool hasGenerics() { return !Generics.empty(); }
  const GenericList &getGenericsList() const { return Generics; }
  void addGenericType(const Position &Pos, llvm::StringRef Identifier) {
    Generics.emplace_back(Pos, Identifier);
  }

  size_t containsGeneric(llvm::StringRef T) {
    for (size_t I = 0; I < Generics.size(); ++I)
      if (Generics[I].Name == T)
        return I;
    return -1;
  }

  void instantiateGeneric(size_t I, type::Type *T) { Generics[I].Type = T; }
  Generic &getGeneric(size_t I) { return Generics[I]; }

  void setModifier(Modifier M) { Modifiers |= M; }
  bool hasModifier() { return Modifiers != 0; }
  bool isPtr() { return (Modifiers & Ptr) == Ptr; }
  bool isRef() { return (Modifiers & Ref) == Ref; }
  bool isOut() { return (Modifiers & Out) == Out; }
  bool isIn() { return (Modifiers & In) == In; }
};

} // namespace north::ast

#endif // LIBNORTH_AST_DECLARATIONS_GENERICDECL_H
