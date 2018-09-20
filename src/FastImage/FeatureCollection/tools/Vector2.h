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


#ifndef VECTOR2_H_
#define VECTOR2_H_

// ================================================================

#include <cstdio>
#include <cmath>
#include <cstring>
#include <cassert>
#include <ostream>

namespace fc {
/// \namespace fc FeatureCollection namespace

/// Represents a 3-dimensional vector.
///
template<class eleT>
class Vector2 {

 private:
  void swap(eleT &a, eleT &b);

  eleT invSqrt(eleT x);

  eleT components[2];

 public:
  Vector2();

  Vector2(const Vector2<eleT> &b);

  Vector2(eleT newX, eleT newY);

  eleT getX() const;

  eleT getY() const;

  eleT getI() const;

  eleT getJ() const;

  void getXY(eleT &newX, eleT &newY) const;

  eleT get(int dimension) const;

  eleT getMin() const;

  eleT getMax() const;

  eleT dot(const Vector2<eleT> &b) const;

  eleT getMagnitude() const;

  eleT getMagnitudeSqr() const;

  eleT getL1Magnitude() const;

  eleT getL1MagnitudeSqr() const;

  void setX(eleT newX);

  void setY(eleT newY);

  void setI(eleT newI);

  void setJ(eleT newJ);

  void setXY(eleT newX, eleT newY);

  void setXY(eleT *newComponents);

  void set(int dimension, eleT value);

  void capBelow(const Vector2<eleT> &minValue);

  void capAbove(const Vector2<eleT> &maxValue);

  void normalize();

  Vector2<eleT> normalized() const;

  void translate(eleT deltaX, eleT deltaY);

//	void rotate(eleT cosTheta, eleT sinTheta, const Vector2<eleT> & axis);

  //in [col][row] format
  void leftMatrixMult2x2(eleT matrix[3][3]);

  void sort();

  Vector2<eleT> roundDown() const;

  Vector2<eleT> roundUp() const;

  void add(int dimension, eleT b);

  void mult(int dimension, eleT b);

  eleT &operator[](std::size_t idx);

  eleT const &operator[](std::size_t idx) const;

  template<class otherEleT>
  Vector2<eleT>
  operator+(const Vector2<otherEleT> &b) const;

  template<class otherEleT>
  Vector2<eleT>
  operator-(const Vector2<otherEleT> &b) const;

  template<class otherEleT>
  Vector2<eleT>
  operator*(const Vector2<otherEleT> &b) const;

  template<class otherEleT>
  Vector2<eleT>
  operator/(const Vector2<otherEleT> &b) const;

  Vector2<eleT> operator+(eleT b) const;

  Vector2<eleT> operator-(eleT b) const;

  Vector2<eleT> operator*(eleT b) const;

  Vector2<eleT> operator/(eleT b) const;

  Vector2<eleT> &operator+=(const Vector2<eleT> &rhs);

  Vector2<eleT> &operator-=(const Vector2<eleT> &rhs);

  Vector2<eleT> &operator*=(const Vector2<eleT> &rhs);

  Vector2<eleT> &operator/=(const Vector2<eleT> &rhs);

  Vector2<eleT> &operator+=(eleT b);

  Vector2<eleT> &operator-=(eleT b);

  Vector2<eleT> &operator*=(eleT b);

  Vector2<eleT> &operator/=(eleT b);

  template<class otherEleT>
  Vector2<eleT> &operator=(const Vector2<otherEleT> &rhs);

  template<class otherEleT>
  bool operator==(const Vector2<otherEleT> &rhs) const;

};

template<class eleT>
std::ostream &operator<<(std::ostream &os, const Vector2<eleT> &rhs) {
  os << "< "
      << rhs.getX() << " , " << rhs.getY()
      << " >";

  return os;
}

template<class eleT>
Vector2<eleT>::Vector2() {
  setXY(0, 0);
}

template<class eleT>
Vector2<eleT>::Vector2(const Vector2<eleT> &b) {
  setXY(b.getX(), b.getY());
}

template<class eleT>
Vector2<eleT>::Vector2(eleT newX, eleT newY) {
  setXY(newX, newY);
}

template<class eleT>
eleT Vector2<eleT>::getX() const {
  return components[0];
}

template<class eleT>
eleT Vector2<eleT>::getY() const {
  return components[1];
}

template<class eleT>
eleT Vector2<eleT>::getI() const {
  return getX();
}

template<class eleT>
eleT Vector2<eleT>::getJ() const {
  return getY();
}

template<class eleT>
void Vector2<eleT>::getXY(eleT &newX, eleT &newY) const {
  newX = getX();
  newY = getY();
}

template<class eleT>
eleT Vector2<eleT>::get(int dimension) const {
  return components[dimension];
}

template<class eleT>
eleT Vector2<eleT>::getMin() const {
  if (get(0) < get(1)) {
    if (get(0) < get(2)) {
      return get(0);
    } else {
      return get(2);
    }
  } else {
    if (get(1) < get(2)) {
      return get(1);
    } else {
      return get(2);
    }
  }
}

template<class eleT>
eleT Vector2<eleT>::getMax() const {
  if (get(0) > get(1)) {
    if (get(0) > get(2)) {
      return get(0);
    } else {
      return get(2);
    }
  } else {
    if (get(1) > get(2)) {
      return get(1);
    } else {
      return get(2);
    }
  }
}

template<class eleT>
eleT Vector2<eleT>::getMagnitude() const {
  return sqrt(getX() * getX() + getY() * getY());
}

template<class eleT>
eleT Vector2<eleT>::getMagnitudeSqr() const {
  return getX() * getX() + getY() * getY();
}

template<class eleT>
eleT Vector2<eleT>::getL1Magnitude() const {
  return fabs(getX()) + fabs(getY());
}

template<class eleT>
eleT Vector2<eleT>::getL1MagnitudeSqr() const {
  return pow(fabs(getX()) + fabs(getY()), 2);
}

template<class eleT>
void Vector2<eleT>::setX(eleT newX) {
  components[0] = newX;
}

template<class eleT>
void Vector2<eleT>::setY(eleT newY) {
  components[1] = newY;
}

template<class eleT>
void Vector2<eleT>::setI(eleT newI) {
  setX(newI);
}

template<class eleT>
void Vector2<eleT>::setJ(eleT newJ) {
  setY(newJ);
}

template<class eleT>
void Vector2<eleT>::setXY(eleT newX, eleT newY) {
  setX(newX);
  setY(newY);
}

template<class eleT>
void Vector2<eleT>::setXY(eleT *newComponents) {
  memcpy(components, newComponents, 2 * sizeof(eleT));
}

template<class eleT>
void Vector2<eleT>::set(int dimension, eleT value) {
  components[dimension] = value;
}

template<class eleT>
Vector2<eleT> Vector2<eleT>::roundDown() const {
  return Vector2<eleT>(floor(getX()), floor(getY()));
}

template<class eleT>
Vector2<eleT> Vector2<eleT>::roundUp() const {
  return Vector2<eleT>(ceil(getX()), ceil(getY()));
}

template<class eleT>
void Vector2<eleT>::add(int dimension, eleT b) {
  set(dimension, get(dimension) + b);
}

template<class eleT>
void Vector2<eleT>::mult(int dimension, eleT b) {
  set(dimension, get(dimension) * b);
}

template<class eleT>
eleT &
Vector2<eleT>::operator[](std::size_t idx) {
  assert(idx >= 0 && idx < 3);

  return components[idx];
}

template<class eleT>
eleT const &
Vector2<eleT>::operator[](std::size_t idx) const {
  assert(idx >= 0 && idx < 3);

  return components[idx];
}

template<class eleT>
template<class otherEleT>
Vector2<eleT> Vector2<eleT>::operator+(const Vector2<otherEleT> &b) const {
  Vector2<eleT> result(getX() + b.getX(), getY() + b.getY());

  return result;
}

template<class eleT>
template<class otherEleT>
Vector2<eleT> Vector2<eleT>::operator-(const Vector2<otherEleT> &b) const {
  Vector2<eleT> result(getX() - b.getX(), getY() - b.getY());

  return result;
}

template<class eleT>
template<class otherEleT>
Vector2<eleT> Vector2<eleT>::operator*(const Vector2<otherEleT> &b) const {
  Vector2<eleT> result(getX() * b.getX(), getY() * b.getY());

  return result;
}

template<class eleT>
template<class otherEleT>
Vector2<eleT> Vector2<eleT>::operator/(const Vector2<otherEleT> &b) const {
  Vector2<eleT> result(getX() / b.getX(), getY() / b.getY());

  return result;
}

template<class eleT>
Vector2<eleT> Vector2<eleT>::operator+(eleT b) const {
  Vector2<eleT> result(getX() + b, getY() + b);

  return result;
}

template<class eleT>
Vector2<eleT> Vector2<eleT>::operator-(eleT b) const {
  Vector2<eleT> result(getX() - b, getY() - b);

  return result;
}

template<class eleT>
Vector2<eleT> Vector2<eleT>::operator*(eleT b) const {
  Vector2<eleT> result(getX() * b, getY() * b);

  return result;
}

template<class eleT>
Vector2<eleT> Vector2<eleT>::operator/(eleT b) const {
  Vector2<eleT> result(getX() / b, getY() / b);

  return result;
}

template<class eleT>
Vector2<eleT> operator+(eleT a, Vector2<eleT> b) {
  Vector2<eleT> result(a + b.getX(), a + b.getY());

  return result;
}

template<class eleT>
Vector2<eleT> operator-(eleT a, Vector2<eleT> b) {
  Vector2<eleT> result(a - b.getX(), a - b.getY());

  return result;
}

template<class eleT>
Vector2<eleT> operator*(eleT a, Vector2<eleT> b) {
  Vector2<eleT> result(a * b.getX(), a * b.getY());

  return result;
}

template<class eleT>
Vector2<eleT> operator/(eleT a, Vector2<eleT> b) {
  Vector2<eleT> result(a / b.getX(), a / b.getY());

  return result;
}

template<class eleT>
Vector2<eleT> &Vector2<eleT>::operator+=(const Vector2<eleT> &rhs) {
  setXY(getX() + rhs.getX(), getY() + rhs.getY());

  return *this;
}

template<class eleT>
Vector2<eleT> &Vector2<eleT>::operator-=(const Vector2<eleT> &rhs) {
  setXY(getX() - rhs.getX(), getY() - rhs.getY());

  return *this;
}

template<class eleT>
Vector2<eleT> &Vector2<eleT>::operator*=(const Vector2<eleT> &rhs) {
  setXY(getX() * rhs.getX(), getY() * rhs.getY());

  return *this;
}

template<class eleT>
Vector2<eleT> &Vector2<eleT>::operator/=(const Vector2<eleT> &rhs) {
  setXY(getX() / rhs.getX(), getY() / rhs.getY());

  return *this;
}

template<class eleT>
Vector2<eleT> &Vector2<eleT>::operator+=(eleT b) {
  setXY(getX() + b, getY() + b);

  return *this;
}

template<class eleT>
Vector2<eleT> &Vector2<eleT>::operator-=(eleT b) {
  setXY(getX() - b, getY() - b);

  return *this;
}

template<class eleT>
Vector2<eleT> &Vector2<eleT>::operator*=(eleT b) {
  setXY(getX() * b, getY() * b);

  return *this;
}

template<class eleT>
Vector2<eleT> &Vector2<eleT>::operator/=(eleT b) {
  setXY(getX() / b, getY() / b);

  return *this;
}

template<class eleT>
template<class otherEleT>
Vector2<eleT> &Vector2<eleT>::operator=(const Vector2<otherEleT> &rhs) {
  setXY((eleT) rhs.getX(), (eleT) rhs.getY());

  return *this;
}

template<class eleT>
template<class otherEleT>
bool Vector2<eleT>::operator==(const Vector2<otherEleT> &rhs) const {
  return ((getX() == rhs.getX()) && (getY() == rhs.getY()));
}

template<class eleT>
void Vector2<eleT>::translate(eleT deltaX, eleT deltaY) {
  setXY(getX() + deltaX, getY() + deltaY);
}

template<class eleT>
eleT Vector2<eleT>::dot(const Vector2<eleT> &b) const {
  return (getX() * b.getX() + getY() * b.getY());
}

template<class eleT>
void Vector2<eleT>::capBelow(const Vector2<eleT> &minValue) {
  if (getX() < minValue.getX()) setX(minValue.getX());
  if (getY() < minValue.getY()) setY(minValue.getY());
}

template<class eleT>
void Vector2<eleT>::capAbove(const Vector2<eleT> &maxValue) {
  if (getX() > maxValue.getX()) setX(maxValue.getX());
  if (getY() > maxValue.getY()) setY(maxValue.getY());
}

template<class eleT>
void Vector2<eleT>::normalize() {
  eleT magnitudeSqr = getMagnitudeSqr();

  if (magnitudeSqr == 0)
    return;

  eleT invMagnitude = invSqrt(magnitudeSqr);

  setXY(getX() * invMagnitude, getY() * invMagnitude);
}

template<class eleT>
Vector2<eleT> Vector2<eleT>::normalized() const {
  Vector2<eleT> normalized(*this);
  normalized.normalize();

  return normalized;
}

//in [col][row] format
template<class eleT>
void Vector2<eleT>::leftMatrixMult2x2(eleT matrix[3][3]) {
  eleT _x = getX();
  eleT _y = getY();

  setX(_x * matrix[0][0] + _y * matrix[1][0]);
  setY(_x * matrix[0][1] + _y * matrix[1][1]);
}

template<class eleT>
void Vector2<eleT>::sort() {
  if (components[1] < components[0])
    swap(components[0], components[1]);

  if (components[2] < components[0])
    swap(components[0], components[2]);

  if (components[2] < components[1])
    swap(components[1], components[2]);
}

template<class eleT>
void Vector2<eleT>::swap(eleT &a, eleT &b) {
  eleT temp = a;
  a = b;
  b = temp;
}

template<class eleT>
eleT Vector2<eleT>::invSqrt(eleT x) {
  return 1. / sqrt(x);
}
}

#endif  // #ifndef VECTOR2_H_

