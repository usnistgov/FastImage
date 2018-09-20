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
// Created: Thu Nov 13 12:27:37 2014 EDT
//
// ================================================================

#ifndef CUBOID_H
#define CUBOID_H

#include <cmath>
#include <ostream>
#include <algorithm>

#include "Vector2.h"

namespace fc {
/// \namespace fc FeatureCollection namespace

/// Represents a cuboid.
///
template<class T>
class Cuboid {
 private:
  Vector2<T> _maxCoords;
  Vector2<T> _minCoords;
 public:
  Cuboid();

  Cuboid(Cuboid<T> const &b);

  Cuboid(Vector2<T> const &minCoords, Vector2<T> const &maxCoords);

  void setMaxCoords(Vector2<T> const &maxCoords);

  Vector2<T> getMaxCoords() const;

  void setMinCoords(Vector2<T> const &minCoords);

  Vector2<T> getMinCoords() const;

  void setMaxCoord(int dim, T maxCoord);

  T getMaxCoord(int dim) const;

  void setMinCoord(int dim, T minCoord);

  T getMinCoord(int dim) const;

  Vector2<T> getCenter() const;

  T getVolume() const;

  T getDistanceTo(Vector2<T> const &point) const;

  T getDistanceSqrTo(Vector2<T> const &point) const;

  Vector2<T> getFarthestPoint(Vector2<T> const &queryPoint) const;

  bool contains(Vector2<T> const &point) const;

};

template<class T>
std::ostream &operator<<(std::ostream &os, const Cuboid<T> &rhs) {
  os << rhs.getMinCoords() << ", " << rhs.getMaxCoords();

  return os;
}

template<class T>
Cuboid<T>::Cuboid()
    : _maxCoords(),
      _minCoords() {

}

template<class T>
Cuboid<T>::Cuboid(Cuboid<T> const &b)
    : _maxCoords(b._maxCoords),
      _minCoords(b._minCoords) {

}

template<class T>
Cuboid<T>::Cuboid(Vector2<T> const &minCoords, Vector2<T> const &maxCoords)
    : _maxCoords(maxCoords),
      _minCoords(minCoords) {

}

template<class T>
void
Cuboid<T>::setMaxCoords(Vector2<T> const &maxCoords) {
  this->_maxCoords = maxCoords;
}

template<class T>
Vector2<T>
Cuboid<T>::getMaxCoords() const {
  return _maxCoords;
}

template<class T>
void
Cuboid<T>::setMinCoords(Vector2<T> const &minCoords) {
  this->_minCoords = minCoords;
}

template<class T>
Vector2<T>
Cuboid<T>::getMinCoords() const {
  return _minCoords;
}

/// Sets the maximum coordinate value of the cuboid along the
/// given dimension.
template<class T>
void
Cuboid<T>::setMaxCoord(int dim, T maxCoord) {
  _maxCoords[dim] = maxCoord;
}

/// Returns the maximum coordinate value the cuboid along the
/// given dimension.
template<class T>
T
Cuboid<T>::getMaxCoord(int dim) const {
  return _maxCoords[dim];
}

/// Sets the minimum coordinate value of the cuboid along the
/// given dimension.
template<class T>
void
Cuboid<T>::setMinCoord(int dim, T minCoord) {
  _minCoords[dim] = minCoord;
}

/// Returns the minimum coordinate value of the cuboid along the
/// given dimension.
template<class T>
T
Cuboid<T>::getMinCoord(int dim) const {
  return _minCoords[dim];
}

template<class T>
Vector2<T>
Cuboid<T>::getCenter() const {
  return (_maxCoords + _minCoords) / 2;
}

template<class T>
T
Cuboid<T>::getVolume() const {
  return (_maxCoords.getX() - _minCoords.getX())
      * (_maxCoords.getY() - _minCoords.getY());
}

/// Returns the distance from the point to the surface of the cuboid.
/// Returns 0 if the point is inside the cuboid.
///
template<class T>
T
Cuboid<T>::getDistanceTo(Vector2<T> const &point) const {
  return sqrt(getDistanceSqrTo(point));
}

/// Returns the distance squared from the point to the surface of the cuboid.
/// Returns 0 if the point is inside the cuboid.
///
template<class T>
T
Cuboid<T>::getDistanceSqrTo(Vector2<T> const &point) const {
  T distanceSqr = 0;

  for (int dim = 0; dim < 2; ++dim) {
    T coordDistance = std::max(_minCoords[dim] - point[dim],
                               point[dim] - _maxCoords[dim]);

    coordDistance = std::max(coordDistance, (T) 0);

    distanceSqr += pow(coordDistance, 2);
  }

  return distanceSqr;
}

/// Returns the point on the surface of the cuboid that is farthest from the
/// given query point.
template<class T>
Vector2<T>
Cuboid<T>::getFarthestPoint(Vector2<T> const &queryPoint) const {
  Vector2<T> farthestPoint;

  Vector2<T> center = (_minCoords + _maxCoords) * 0.5;

  for (int dim = 0; dim < 2; ++dim) {
    if (queryPoint[dim] < center[dim]) {
      farthestPoint[dim] = _maxCoords[dim];
    } else {
      farthestPoint[dim] = _minCoords[dim];
    }
  }

  return farthestPoint;
}

/// Returns whether the cuboid encloses the point.
template<class T>
bool
Cuboid<T>::contains(Vector2<T> const &point) const {
  for (int dim = 0; dim < 2; ++dim) {
    if (_minCoords[dim] > point[dim] ||
        _maxCoords[dim] < point[dim]) {

      return false;
    }
  }

  return true;
}

}

#endif

