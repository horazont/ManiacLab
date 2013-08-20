/**********************************************************************
File name: Operation.hpp
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
#ifndef _ML_OPERATION_H
#define _ML_OPERATION_H

#include <stdexcept>
#include <memory>
#include <vector>

class OperationNotUndoable: public std::logic_error
{
public:
    OperationNotUndoable(const std::string &what_arg);
    OperationNotUndoable(const char *what_arg);
    OperationNotUndoable(const OperationNotUndoable &ref) = default;
    OperationNotUndoable& operator=(
        const OperationNotUndoable &ref) = default;
};

class Operation
{
public:
    Operation();
    Operation(const Operation &ref);
    Operation& operator=(const Operation &ref);
    virtual ~Operation();

protected:
    void not_undoable();

public:
    virtual void execute() = 0;
    virtual bool is_undoable();
    virtual void undo() = 0;

};

typedef std::unique_ptr<Operation> OperationPtr;

class OperationGroup: public Operation
{
public:
    OperationGroup();
    OperationGroup(const OperationGroup &ref) = delete;
    OperationGroup& operator=(const OperationGroup &ref) = delete;

private:
    std::vector<OperationPtr> _operations;

public:
    void execute() override;
    bool is_undoable() override;
    void undo() override;

public:
    void add_operation(OperationPtr &&operation);
    bool empty();

};

#endif
