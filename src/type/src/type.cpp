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
//     * Neither the name of Terifo nor the names of its contributors
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

#include "type.h"
#include <algorithm>
#include <cstdint>
#include <stdexcept>

namespace swallow::type
{
  std::string TypeManager::newTypeName() noexcept
  {
    int32_t currentID = LastID++;
    std::string name = "";

    while (LastID != -1)
      {
        name += static_cast<char>(('a' + (currentID % 26)));
        currentID = currentID / 26 - 1;
      }

    std::reverse(name.begin(), name.end());
    return name;
  }

  Type::Ptr TypeManager::newType() noexcept
  {
    return Type::Ptr(new TypeVar(newTypeName()));
  }

  Type::Ptr TypeManager::newArrowType() noexcept
  {
    return Type::Ptr(new TypeArrow(newType(), newType()));
  }

  Type::Ptr TypeManager::resolve(Type::Ptr type, TypeVar *& var) noexcept
  {
    TypeVar * cast = nullptr;
    var = nullptr;

    while ((cast = dynamic_cast<TypeVar *>(type.get())))
      {
        const auto it = Types.find(cast->Name);
        if (it == Types.end())
          {
            var = cast;
            break;
          }

        type = it->second;
      }

    return type;
  }

  void TypeManager::unify(Type::Ptr left, Type::Ptr right) noexcept
  {
    TypeVar * leftVar = nullptr;
    TypeVar * rightVar = nullptr;
    TypeArrow * leftArrow = nullptr;
    TypeArrow * rightArrow = nullptr;
    TypeBase * leftID = nullptr;
    TypeBase * rightID = nullptr;

    left = resolve(left, leftVar);
    right = resolve(right, rightVar);

    if (leftVar)
      {
        bind(leftVar->Name, right);
        return;
      }
    else if (rightVar)
      {
        bind(rightVar->Name, left);
        return;
      }
    else if ((leftArrow = dynamic_cast<TypeArrow *>(left.get()))
             && (rightArrow = dynamic_cast<TypeArrow *>(right.get())))
      {
        unify(leftArrow->Left, rightArrow->Left);
        unify(leftArrow->Right, rightArrow->Right);
        return;
      }
    else if ((leftID = dynamic_cast<TypeBase *>(left.get()))
             && (rightID = dynamic_cast<TypeBase *>(right.get())))
      {
        if (leftID->Name == rightID->Name)
          return;
      }

    throw std::runtime_error("type checking error!!!");
  }

  void TypeManager::bind(const std::string & name, Type::Ptr type) noexcept
  {
    TypeVar * other = dynamic_cast<TypeVar *>(type.get());
    if (other && other->Name == name)
      return;
    Types[name] = type;
  }

} // namespace swallow::type