// NIST-developed software is provided by NIST as a public service. 
// You may use, copy and distribute copies of the  software in any  medium, 
// provided that you keep intact this entire notice. You may improve, 
// modify and create derivative works of the software or any portion of the 
// software, and you may copy and distribute such modifications or works. 
// Modified works should carry a notice stating that you changed the software 
// and should note the date and nature of any such change. Please explicitly 
// acknowledge the National Institute of Standards and Technology as the 
// source of the software.
// NIST-developed software is expressly provided "AS IS." NIST MAKES NO WARRANTY
// OF ANY KIND, EXPRESS, IMPLIED, IN FACT  OR ARISING BY OPERATION OF LAW, 
// INCLUDING, WITHOUT LIMITATION, THE IMPLIED WARRANTY OF MERCHANTABILITY, 
// FITNESS FOR A PARTICULAR PURPOSE, NON-INFRINGEMENT AND DATA ACCURACY. NIST 
// NEITHER REPRESENTS NOR WARRANTS THAT THE OPERATION  OF THE SOFTWARE WILL 
// BE UNINTERRUPTED OR ERROR-FREE, OR THAT ANY DEFECTS WILL BE CORRECTED. NIST 
// DOES NOT WARRANT  OR MAKE ANY REPRESENTATIONS REGARDING THE USE OF THE 
// SOFTWARE OR THE RESULTS THEREOF, INCLUDING BUT NOT LIMITED TO THE 
// CORRECTNESS, ACCURACY, RELIABILITY, OR USEFULNESS OF THE SOFTWARE.
// You are solely responsible for determining the appropriateness of using 
// and distributing the software and you assume  all risks associated with 
// its use, including but not limited to the risks and costs of program 
// errors, compliance  with applicable laws, damage to or loss of data, 
// programs or equipment, and the unavailability or interruption of operation. 
// This software is not intended to be used in any situation where a failure 
// could cause risk of injury or damage to property. The software developed 
// by NIST employees is not subject to copyright protection within 
// the United States.

/// @file UnionFind.h
/// @author Alexandre Bardakoff - Timothy Blattner
/// @date  7/25/17
/// @brief Union FInd algorithm


#ifndef REGIONLABELING_UNIONFIND_H
#define REGIONLABELING_UNIONFIND_H

#include <set>
#include <list>
#include "FastImage/FeatureCollection/Data/Blob.h"

namespace fc {
/// \namespace fc FeatureCollection namespace

/// \class UnionFind UnionFind.h <FastImage/FeatureCollection/tools/UnionFind.h>
/// \brief Templatized Union-Find algorithm as described
/// https://en.wikipedia.org/wiki/Disjoint-set_data_structure
/// \tparam T File type
template<class T>
class UnionFind {
 public:
  /// \brief Find the element parent, and compress the path
  /// \param elem Element to look for the parent
  /// \return this if has not parent or parent address
  T *find(T *elem) {
    if (elem->getParent() != elem) {
      elem->setParent(find(elem->getParent()));
    }
    return elem->getParent();
  }

  /// \brief Unify to elements
  /// \param elem1 element 1 to unify
  /// \param elem2 element 2 to unify
  void unionElements(T *elem1, T *elem2) {
    auto root1 = find(elem1);
    auto root2 = find(elem2);
    if (root1 != root2) {
      if (root1->getRank() < root2->getRank()) {
        root1->setParent(root2);
      } else {
        root2->setParent(root1);
        if (root1->getRank() == root2->getRank()) {
          root1->setRank(root1->getRank() + 1);
        }
      }
    }
  }

};
}

#endif //REGIONLABELING_UNIONFIND_H
