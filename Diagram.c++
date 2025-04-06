// Copyright (C) 2002, 2003
// Charles Gretton and David Price
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

/* Diagram.c++
 *
 * \paragraph{:purpose:}
 *
 * Ties together information needed to produce a diagram of an ADD
 * into a single container together with methods to output such a
 * diagram.
 */

#include "common.h++"
#ifdef HEADER

#include "ADDvisitor.h++"
#include "ExtADD.h++"

#include <vector>
#include <string>
#include <set>
#include <map>
using namespace std;

class Diagram : public ADDvisitor
{
private:
    vector<ExtADD> contents;
    vector<string> titles;
    string dotString;
    vector<string> nodeNames;
    vector<string> leafNames;
    
    set<string> nodesVisited;

    map<int, int> nodeIDs;
    int nextNodeID;
};

#endif
#include "Diagram.h++"

PUBLIC Diagram::Diagram()
    : nextNodeID(0)
{
}

PUBLIC VIRTUAL Diagram::~Diagram()
{
}

PUBLIC void Diagram::setNodeNames(const vector<string>& nodeNames)
{
    this->nodeNames = nodeNames;
}

/* Associates names with leaves, where leaves should be integers that
 * correspond to valid indexes in the leafNames array.  If leafNames
 * is empty (default), then the leaves are just their values.
 */
PUBLIC void Diagram::setLeafNames(const vector<string>& leafNames)
{
    this->leafNames = leafNames;
}

PUBLIC void Diagram::addPage(ExtADD newADD, const string& title)
{
    contents.push_back(newADD);
    titles.push_back(title);
}

PUBLIC string Diagram::toDotString()
{
    dotString = "";
    int num_pages = contents.size();
    for (int i = 0; i < num_pages; i++) {
        dotString +=
            "digraph \"" + titles[i] + "\" {\n"
            "center = true;\n"
            "edge [dir = none];\n"
            "titlenode [shape=box color=white label=\"" + titles[i] + "\"];\n";
        nodesVisited.clear();
        contents[i].traverse(*this);
        dotString += "}\n";
    }
    return dotString;
}

/* Translate ADD unique IDs into a different ID, that will be the same
 * every time the program is run - provided the input is the same.  We
 * do this in order to make regression testing possible. */
PRIVATE string Diagram::getNodeID(const ExtADD& node)
{
    if (0 == nodeIDs.count(node.getID())) {
        nodeIDs[node.getID()] = nextNodeID;
        nextNodeID++;
    }

    return "n" + Utils::intToString(nodeIDs[node.getID()]);
}

PUBLIC void Diagram::visitInternal(const ExtADD& node)
{
    // we only want to examine each node once
    if (nodesVisited.count(getNodeID(node)) == 0) {
        nodesVisited.insert(getNodeID(node));
        
        string nodeName;
        if (!nodeNames.empty()) {
            if (node.isPrimed(nodeNames.size()))
                nodeName = nodeNames[node.getIndex() - nodeNames.size()] + "'";
            else
                nodeName = nodeNames[node.getIndex()];
        }
        bool showThen = true, showElse = true;
        if (!leafNames.empty()) {
            if (node.thenChild().isConstant()) {
                if (leafNames[int(node.thenChild().value())] == "__NOSHOW__")
                    showThen = false;
            }
            if (node.elseChild().isConstant()) {
                if (leafNames[int(node.elseChild().value())] == "__NOSHOW__")
                    showElse = false;
            }
        }
        dotString += getNodeID(node) + " [label=\""
            + nodeName + "\"];\n";
        if (showThen) {
            dotString += getNodeID(node) + " -> " + getNodeID(node.thenChild()) + ";\n";
        }
        if (showElse) {
            dotString += getNodeID(node) + " -> " + getNodeID(node.elseChild()) + " [style=dashed]\n";
        }
    }
}

PUBLIC void Diagram::visitLeaf(const ExtADD& node)
{
    if (nodesVisited.count(getNodeID(node)) == 0) {
        nodesVisited.insert(getNodeID(node));
        string name;
        if (leafNames.empty()) {
            name = Utils::doubleToString(node.value(), 2);
        } else {
            name = leafNames[int(node.value())];
        }
        if (name != "__NOSHOW__") {
            dotString += getNodeID(node)
                + " [label=\"" + name + "\" shape=box];\n";
        }
    }
}
