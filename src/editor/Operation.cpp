/**********************************************************************
File name: Operation.cpp
This file is part of: ManiacLab

LICENSE

This program is free software: you can redistribute it and/or modify it
under the terms of the GNU General Public License as published by the
Free Software Foundation, either version 3 of the License, or (at your
option) any later version.

This program is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
General Public License for more details.

You should have received a copy of the GNU General Public License along
with this program.  If not, see <http://www.gnu.org/licenses/>.

FEEDBACK & QUESTIONS

For feedback and questions about ManiacLab please e-mail one of the
authors named in the AUTHORS file.
**********************************************************************/
#include "Operation.hpp"

/* OperationNotUndoable */

OperationNotUndoable::OperationNotUndoable(const std::string &what_arg):
    std::logic_error(what_arg)
{

}

OperationNotUndoable::OperationNotUndoable(const char *what_arg):
    std::logic_error(what_arg)
{

}

/* Operation */

Operation::Operation()
{

}

Operation::Operation(const Operation &ref)
{

}

Operation& Operation::operator=(const Operation &ref)
{
    return *this;
}

Operation::~Operation()
{

}

void Operation::execute()
{

}

bool Operation::is_undoable()
{
    return false;
}

void Operation::undo()
{
    throw OperationNotUndoable("This operation cannot be un-done.");
}
