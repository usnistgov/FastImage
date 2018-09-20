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

// ================================================================
//
// Authors: Derek Juba <derek.juba@nist.gov>
// Created: Fri Feb 10 11:41:46 2017 EDT
//
// ================================================================

#ifndef AABB_TREE_H
#define AABB_TREE_H

#include <vector>
#include <algorithm>
#include <cstdint>

#include "Vector2.h"
#include "Cuboid.h"

// ================================================================

namespace fc {
/// \namespace fc FeatureCollection namespace

/// Spatial data structure for nearest-object queries based on an
/// Axis Aligned Bounding Box (AABB) tree.  The data structure can store any
/// objects which themselves can be enclosed in an AABB and which can compute
/// their distance to a point.  Specifically, the objects must provide the
/// following functions:
///
/// double getMaxCoord(int dim)
/// double getMinCoord(int dim)
/// double getDistanceSqrTo(Vector2<double> const & point)
/// bool contains(Vector2<double> const & point)
///
template<class ObjectT>
class AABBTree {
 private:
  using objects_size_type  = typename std::vector<ObjectT>::size_type;
  struct AABBNode {
    Cuboid<double> aabb;

    objects_size_type objectsBegin;
    objects_size_type objectsEnd;
  };
  using aabbTree_size_type = typename std::vector<AABBNode>::size_type;

  const objects_size_type maxNodeSize;

  std::vector<AABBNode> aabbTree;

  std::vector<ObjectT> *sortedObjects;

  void computeAABBBounds(aabbTree_size_type nodeIndex);

  void splitAABBNode(aabbTree_size_type nodeIndex,
                     int splitDim);

 public:
  explicit AABBTree(objects_size_type maxNodeSize = 4);

  ~AABBTree();

  void preprocess(std::vector<ObjectT> *objects);

  std::vector<ObjectT *> objectsContain(Vector2<double> const &queryPoint)
  const;
};

// ================================================================

/// Constructs an empty spatial data structure.
///
template<class ObjectT>
AABBTree<ObjectT>::AABBTree(objects_size_type maxNodeSize)
    : maxNodeSize(maxNodeSize),
      aabbTree(),
      sortedObjects(NULL) {

}

template<class ObjectT>
AABBTree<ObjectT>::~AABBTree() = default;

/// Inserts the given objects into the spatial data structure.
/// The objects vector may be modified and must not be changed outside the
/// class.
///
template<class ObjectT>
void
AABBTree<ObjectT>::preprocess(std::vector<ObjectT> *objects) {

  aabbTree.clear();

  sortedObjects = objects;

  if (objects->empty()) {
    return;
  }

  aabbTree.resize(1);

  aabbTree.at(0).objectsBegin = 0;
  aabbTree.at(0).objectsEnd = sortedObjects->size();

  aabbTree_size_type nodesBegin = 0;
  aabbTree_size_type nodesEnd = 1;

  int splitDim = 0;

  bool splitOccurred = true;

  while (splitOccurred) {
    splitOccurred = false;

    for (aabbTree_size_type nodeIndex = nodesBegin;
         nodeIndex < nodesEnd;
         ++nodeIndex) {

      computeAABBBounds(nodeIndex);

      objects_size_type nodeSize =
          aabbTree.at(nodeIndex).objectsEnd -
              aabbTree.at(nodeIndex).objectsBegin;

      if (nodeSize > maxNodeSize) {
        if (!splitOccurred) {
          aabbTree_size_type newAABBTreeSize = aabbTree.size() * 2 + 1;

          aabbTree.resize(newAABBTreeSize);
        }

        splitAABBNode(nodeIndex, splitDim);

        splitOccurred = true;
      }
    }

    nodesBegin = nodesEnd;
    nodesEnd = nodesEnd * 2 + 1;

    ++splitDim;

    if (splitDim == 2) {
      splitDim = 0;
    }
  }
}

template<class ObjectT>
void
AABBTree<ObjectT>::computeAABBBounds(aabbTree_size_type nodeIndex) {
  for (int dim = 0; dim < 2; ++dim) {
    aabbTree.at(nodeIndex).aabb.setMaxCoord(
        dim, std::numeric_limits<double>::lowest());
  }

  for (int dim = 0; dim < 2; ++dim) {
    aabbTree.at(nodeIndex).aabb.setMinCoord(
        dim, std::numeric_limits<double>::max());
  }

  for (objects_size_type objectIndex =
      aabbTree.at(nodeIndex).objectsBegin;
       objectIndex < aabbTree.at(nodeIndex).objectsEnd;
       ++objectIndex) {

    for (int dim = 0; dim < 2; ++dim) {
      aabbTree.at(nodeIndex).aabb.
          setMaxCoord(dim,
                      std::max(
                          aabbTree.at(nodeIndex).aabb.getMaxCoord(dim),
                          sortedObjects->at(objectIndex).getMaxCoord(dim)));
    }

    for (int dim = 0; dim < 2; ++dim) {
      aabbTree.at(nodeIndex).aabb.
          setMinCoord(dim,
                      std::min(
                          aabbTree.at(nodeIndex).aabb.getMinCoord(dim),
                          sortedObjects->at(objectIndex).getMinCoord(dim)));
    }
  }
}

template<class ObjectT>
void
AABBTree<ObjectT>::splitAABBNode(aabbTree_size_type nodeIndex,
                                 int splitDim) {

  class CoordComp {
   public:
    explicit CoordComp(int dim) : dim(dim) {}

    bool operator()(ObjectT i, ObjectT j) {
      double iMidCoordX2 = i.getMaxCoord(dim) + i.getMinCoord(dim);
      double jMidCoordX2 = j.getMaxCoord(dim) + j.getMinCoord(dim);

      return (iMidCoordX2 < jMidCoordX2);
    }

   private:
    int dim;
  };

  CoordComp coordComp(splitDim);

  std::sort(sortedObjects->begin() + aabbTree.at(nodeIndex).objectsBegin,
            sortedObjects->begin() + aabbTree.at(nodeIndex).objectsEnd,
            coordComp);

  objects_size_type nodeSize =
      aabbTree.at(nodeIndex).objectsEnd -
          aabbTree.at(nodeIndex).objectsBegin;

  objects_size_type objectsMiddle =
      aabbTree.at(nodeIndex).objectsBegin +
          nodeSize / 2;

  aabbTree_size_type lowChildIndex = nodeIndex * 2 + 1;
  aabbTree_size_type highChildIndex = nodeIndex * 2 + 2;

  aabbTree.at(lowChildIndex).objectsBegin =
      aabbTree.at(nodeIndex).objectsBegin;

  aabbTree.at(lowChildIndex).objectsEnd =
      objectsMiddle;

  aabbTree.at(highChildIndex).objectsBegin =
      objectsMiddle;

  aabbTree.at(highChildIndex).objectsEnd =
      aabbTree.at(nodeIndex).objectsEnd;

  aabbTree.at(nodeIndex).objectsBegin = 0;
  aabbTree.at(nodeIndex).objectsEnd = 0;
}

/// Returns whether any object in the tree contains the given query point.
///
template<class ObjectT>
std::vector<ObjectT *> AABBTree<ObjectT>::objectsContain(
    Vector2<double> const &queryPoint) const {

  std::vector<ObjectT *> vObj;
  if (aabbTree.empty()) {
    return vObj;
  }

  std::vector<aabbTree_size_type> nodesToCheck;

  nodesToCheck.emplace_back(0);

  while (!nodesToCheck.empty()) {
    aabbTree_size_type nodeIndex = nodesToCheck.back();

    nodesToCheck.pop_back();

    objects_size_type objectsBegin = aabbTree.at(nodeIndex).objectsBegin;
    objects_size_type objectsEnd = aabbTree.at(nodeIndex).objectsEnd;

    if (objectsBegin == objectsEnd) {
      aabbTree_size_type lowChildIndex = nodeIndex * 2 + 1;
      aabbTree_size_type highChildIndex = nodeIndex * 2 + 2;

      if (aabbTree.at(highChildIndex).aabb.contains(queryPoint)) {
        nodesToCheck.emplace_back(highChildIndex);
      }

      if (aabbTree.at(lowChildIndex).aabb.contains(queryPoint)) {
        nodesToCheck.emplace_back(lowChildIndex);
      }
    } else {
      // leaf node
      for (objects_size_type objectIndex = objectsBegin;
           objectIndex < objectsEnd;
           ++objectIndex) {
        auto feature = sortedObjects->at(objectIndex);
        feature.getId();
        if (sortedObjects->at(objectIndex).contains(queryPoint)) {
          vObj.push_back(&sortedObjects->at(objectIndex));
        }
      }
    }
  }
  return vObj;
}
}

#endif
