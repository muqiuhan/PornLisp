// Copyright (c) 2023 Muqiu Han
//
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
//     * Redistributions of source code must retain the above copyright notice,
//       this list of conditions and the following disclaimer.
//     * Redistributions in binary form must reproduce the above copyright
//     notice,
//       this list of conditions and the following disclaimer in the
//       documentation and/or other materials provided with the distribution.
//     * Neither the name of Swallow nor the names of its contributors
//       may be used to endorse or promote products derived from this software
//       without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
// CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
// EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
// PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
// PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
// LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
// NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
// SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#ifndef SWALLOW_COMPILER_AST_AST_H
#define SWALLOW_COMPILER_AST_AST_H

#include "binop/binop.hpp"
#include "g-machine/environment.hpp"
#include "g-machine/instruction.hpp"
#include "parser/location.hh"
#include "result/result.hpp"
#include "type/environment.hpp"
#include "type/type.hpp"
#include <cstdint>
#include <memory>
#include <string>
#include <vector>

namespace swallow::compiler::ast
{
  class AST
  {
  public:
    const yy::location Location;
    type::Type::Ptr NodeType;

    using Ptr = std::unique_ptr<AST>;

    explicit AST(const yy::location Location) : Location(Location) {}
    virtual ~AST() = default;

    auto CommonTypeCheck(type::Manager &typeManager,
                         const type::Environment &typeEnvironment) noexcept -> type::Type::Ptr;

    void CommonResolve(const type::Manager &typeManager) noexcept;

    virtual void Resolve(const type::Manager &typeManager) const noexcept = 0;

    virtual auto TypeCheck(type::Manager &typeManager, const type::Environment &typeEnvironment) const noexcept
      -> utils::Result<type::Type::Ptr, utils::Void> = 0;

    virtual void Dump(uint8_t indent, std::ostream &to) const noexcept = 0;

    virtual void Compile(const gmachine::Environment::Ptr &machineEnvironment,
                         std::vector<gmachine::instruction::Instruction::Ptr> &into) const noexcept = 0;
  };

  class Pattern
  {
  public:
    using Ptr = std::unique_ptr<Pattern>;

    const yy::location Location;

    explicit Pattern(const yy::location Location) : Location(Location) {}
    virtual ~Pattern() = default;

    virtual void Match(type::Type::Ptr type, type::Manager &typeManager,
                       type::Environment &typeEnvironment) const noexcept = 0;

    virtual void Dump(std::ostream &to) const noexcept = 0;
  };

  class Branch
  {
  public:
    using Ptr = std::unique_ptr<Branch>;

    const yy::location Location;
    const Pattern::Ptr Patt;
    const AST::Ptr Expr;

    Branch(const yy::location Location, Pattern::Ptr Patt, AST::Ptr Expr)
      : Patt(std::move(Patt)), Expr(std::move(Expr)), Location(Location)
    {}
  };

  class Constructor
  {
  public:
    using Ptr = std::unique_ptr<Constructor>;

    const std::string Name;
    const std::vector<std::string> Types;
    const yy::location Location;
    uint8_t Tag{};

    Constructor(const yy::location Location, std::string Name, std::vector<std::string> Types)
      : Name(std::move(Name)), Types(std::move(Types)), Location(Location)
    {}
  };

  class Definition
  {
  public:
    using Ptr = std::unique_ptr<Definition>;

    const yy::location Location;

    explicit Definition(const yy::location Location) : Location(Location) {}
    virtual ~Definition() = default;

    virtual void PreScanTypes(type::Manager &typeManager, type::Environment &typeEnvironment) noexcept = 0;

    virtual void Resolve(const type::Manager &typeManager) const noexcept = 0;

    virtual void TypeCheck(type::Manager &typeManager, const type::Environment &typeEnvironment) const noexcept = 0;
  };

  class Int final : public AST
  {
    const int Value;

  public:
    explicit Int(const yy::location Location, const int V) : Value(V), AST(Location) {}

    auto TypeCheck(type::Manager &typeManager, const type::Environment &typeEnvironment) const noexcept
      -> utils::Result<type::Type::Ptr, utils::Void> override;

    void Dump(uint8_t indent, std::ostream &to) const noexcept override;

    void Compile(const gmachine::Environment::Ptr &machineEnvironment,
                 std::vector<gmachine::instruction::Instruction::Ptr> &into) const noexcept override;

    virtual void Resolve(const type::Manager &typeManager) const noexcept override;
  };

  class LID final : public AST
  {
    const std::string ID;

  public:
    explicit LID(const yy::location Location, std::string ID) : ID(std::move(ID)), AST(Location) {}

    auto TypeCheck(type::Manager &typeManager, const type::Environment &typeEnvironment) const noexcept
      -> utils::Result<type::Type::Ptr, utils::Void> override;

    void Dump(uint8_t indent, std::ostream &to) const noexcept override;

    void Compile(const gmachine::Environment::Ptr &machineEnvironment,
                 std::vector<gmachine::instruction::Instruction::Ptr> &into) const noexcept override;
    virtual void Resolve(const type::Manager &typeManager) const noexcept override;
  };

  class UID final : public AST
  {
    const std::string ID;

  public:
    explicit UID(const yy::location Location, std::string ID) : ID(std::move(ID)), AST(Location) {}

    auto TypeCheck(type::Manager &typeManager, const type::Environment &typeEnvironment) const noexcept
      -> utils::Result<type::Type::Ptr, utils::Void> override;

    void Dump(uint8_t indent, std::ostream &to) const noexcept override;

    void Compile(const gmachine::Environment::Ptr &machineEnvironment,
                 std::vector<gmachine::instruction::Instruction::Ptr> &into) const noexcept override;
    virtual void Resolve(const type::Manager &typeManager) const noexcept override;
  };

  class Binop final : public AST
  {
  public:
    const utils::Binop Operator;
    const Ptr Left;
    const Ptr Right;

    Binop(const yy::location Location, utils::Binop Operator, Ptr Left, Ptr Right)
      : Operator(Operator), Left(std::move(Left)), Right(std::move(Right)), AST(Location)
    {}

    static auto OperatorToString(utils::Binop op) noexcept -> std::string;

    auto TypeCheck(type::Manager &typeManager, const type::Environment &typeEnvironment) const noexcept
      -> utils::Result<type::Type::Ptr, utils::Void> override;

    void Dump(uint8_t indent, std::ostream &to) const noexcept override;

    void Compile(const gmachine::Environment::Ptr &machineEnvironment,
                 std::vector<gmachine::instruction::Instruction::Ptr> &into) const noexcept override;

    virtual void Resolve(const type::Manager &typeManager) const noexcept override;
  };

  class Application final : public AST
  {
    const Ptr Left;
    const Ptr Right;

  public:
    Application(const yy::location Location, Ptr Left, Ptr Right)
      : Left(std::move(Left)), Right(std::move(Right)), AST(Location)
    {}

    auto TypeCheck(type::Manager &typeManager, const type::Environment &typeEnvironment) const noexcept
      -> utils::Result<type::Type::Ptr, utils::Void> override;

    void Dump(uint8_t indent, std::ostream &to) const noexcept override;

    void Compile(const gmachine::Environment::Ptr &machineEnvironment,
                 std::vector<gmachine::instruction::Instruction::Ptr> &into) const noexcept override;
    virtual void Resolve(const type::Manager &typeManager) const noexcept override;
  };

  class Match final : public AST
  {
    const Ptr With;
    const std::vector<Branch::Ptr> Branches;

  public:
    Match(const yy::location Location, Ptr o, std::vector<Branch::Ptr> b)
      : With(std::move(o)), Branches(std::move(b)), AST(Location)
    {}

    auto TypeCheck(type::Manager &typeManager, const type::Environment &typeEnvironment) const noexcept
      -> utils::Result<type::Type::Ptr, utils::Void> override;

    void Dump(uint8_t indent, std::ostream &to) const noexcept override;

    void Compile(const gmachine::Environment::Ptr &machineEnvironment,
                 std::vector<gmachine::instruction::Instruction::Ptr> &into) const noexcept override;
    virtual void Resolve(const type::Manager &typeManager) const noexcept override;
  };

  class VariablePattern final : public Pattern
  {
    const std::string Variable;

  public:
    explicit VariablePattern(const yy::location Location, std::string Variable)
      : Variable(std::move(Variable)), Pattern(Location)
    {}

    void Match(type::Type::Ptr type, type::Manager &typeManager,
               type::Environment &typeEnvironment) const noexcept override;

    void Dump(std::ostream &to) const noexcept override;
  };

  class ConstructorPattern final : public Pattern
  {
  public:
    const std::string ConstructorName;
    const std::vector<std::string> Params;

    ConstructorPattern(const yy::location Location, std::string Constructor, std::vector<std::string> Params)
      : ConstructorName(std::move(Constructor)), Params(std::move(Params)), Pattern(Location)
    {}

    void Match(type::Type::Ptr type, type::Manager &typeManager,
               type::Environment &typeEnvironment) const noexcept override;

    void Dump(std::ostream &to) const noexcept override;
  };

  class Fn final : public Definition
  {
  public:
    const std::string Name;
    const std::vector<std::string> Params;
    const AST::Ptr Body;

    std::vector<type::Type::Ptr> ParamTypes;
    type::Type::Ptr ReturnType;

    Fn(const yy::location Location, std::string Name, std::vector<std::string> Params, AST::Ptr Body)
      : Name(std::move(Name)), Params(std::move(Params)), Body(std::move(Body)), Definition(Location)
    {}

    void PreScanTypes(type::Manager &typeManager, type::Environment &typeEnvironment) noexcept override;

    void TypeCheck(type::Manager &typeManager, const type::Environment &typeEnvironment) const noexcept override;
    virtual void Resolve(const type::Manager &typeManager) const noexcept override;
  };

  class Data final : public Definition
  {
  public:
    const std::string Name;
    const std::vector<Constructor::Ptr> Constructors;

    Data(const yy::location Location, std::string Name, std::vector<Constructor::Ptr> Constructors)
      : Name(std::move(Name)), Constructors(std::move(Constructors)), Definition(Location)
    {}

    void PreScanTypes(type::Manager &typeManager, type::Environment &typeEnvironment) noexcept override;
    void TypeCheck(type::Manager &typeManager, const type::Environment &typeEnvironment) const noexcept override;
    virtual void Resolve(const type::Manager &typeManager) const noexcept override;
  };

  void Dump(const std::vector<Definition> &Program) noexcept;
} // namespace swallow::compiler::ast

namespace swallow::compiler::type
{
  void TypeCheck(const std::vector<ast::Definition::Ptr> &program) noexcept;
} // namespace swallow::compiler::type

#endif