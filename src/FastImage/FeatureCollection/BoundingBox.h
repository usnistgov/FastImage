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

/// @file BoundingBox.h
/// @author Alexandre Bardakoff - Timothy Blattner
/// @date  8/9/17
/// @brief Feature Bounding box

#ifndef FEATURECOLLECTION_BOUNDINGBOX_H
#define FEATURECOLLECTION_BOUNDINGBOX_H

#include <fstream>

namespace fc {
/// \namespace fc FeatureCollection namespace

/**
  * @class BoundingBox BoundingBox.h <FastImage/FeatureCollection/BoundingBox.h>
  *
  * @brief Define a bounding box around a feature.
  *
  **/
class BoundingBox {
 public:
  /// \brief Bounding box constructor
  /// \param upperLeftRow Upper left row in global coordinate
  /// \param upperLeftCol Upper left column in global coordinate
  /// \param bottomRightRow Bottom right row in global coordinate
  /// \param bottomRightCol Bottom right column in global coordinate
  explicit BoundingBox(uint32_t upperLeftRow,
                       uint32_t upperLeftCol,
                       uint32_t bottomRightRow,
                       uint32_t bottomRightCol)
      : _upperLeftRow(upperLeftRow), _upperLeftCol(upperLeftCol),
        _bottomRightRow(bottomRightRow), _bottomRightCol(bottomRightCol) {}

  /// \brief Copy constructor
  /// \param src Bounding box to copy
  BoundingBox(const BoundingBox &src) {
    this->_upperLeftRow = src.getUpperLeftRow();
    this->_upperLeftCol = src.getUpperLeftCol();
    this->_bottomRightRow = src.getBottomRightRow();
    this->_bottomRightCol = src.getBottomRightCol();
  }

  /// \brief Getter to the upper left row
  /// \return The upper left row
  uint32_t getUpperLeftRow() const { return _upperLeftRow; }

  /// \brief Getter to the upper left column
  /// \return The upper left column
  uint32_t getUpperLeftCol() const { return _upperLeftCol; }

  /// \brief Getter to the bottom right row
  /// \return The bottom right row
  uint32_t getBottomRightRow() const { return _bottomRightRow; }

  /// \brief Getter to the bottom right column
  /// \return The bottom right column
  uint32_t getBottomRightCol() const { return _bottomRightCol; }

  /// \brief Getter the Bounding Box width
  /// \return The Bounding Box width
  uint32_t getWidth() const { return _bottomRightCol - _upperLeftCol; }

  /// \brief Getter to the Bounding Box height
  /// \return The Bounding Box height
  uint32_t getHeight() const { return _bottomRightRow - _upperLeftRow; }

  /// \brief Getter to the middle row
  /// \return The middle row
  double getMiddleRow() const {
    return (double) (_bottomRightRow - _upperLeftRow) / 2;
  }

  /// \brief Getter to the middle column
  /// \return The middle column
  double getMiddleCol() const {
    return (double) (_bottomRightCol - _upperLeftCol) / 2;
  }

  /// \brief Setter to the upper left row
  /// \param rowTopLeft To the upper left row to set
  void setUpperLeftRow(uint32_t rowTopLeft) { _upperLeftRow = rowTopLeft; }

  /// \brief Setter to the upper left column
  /// \param colTopLeft The upper left column to set
  void setUpperLeftCol(uint32_t colTopLeft) { _upperLeftCol = colTopLeft; }

  /// \brief Setter to the bottom right row
  /// \param bottomRightRow Bottom right row to set
  void setBottomRightRow(uint32_t bottomRightRow) {
    _bottomRightRow = bottomRightRow;
  }

  /// \brief Setter to the bottom right column
  /// \param bottomRightCol Bottom right column to set
  void setBottomRightCol(uint32_t bottomRightCol) {
    _bottomRightCol = bottomRightCol;
  }

  /// \brief Serialize a Bounding box into a output stream
  /// \param outFile output stream to put the bounding box serial
  void serializeBoundingBox(std::ofstream &outFile) {
    outFile << this->_upperLeftRow << " ";
    outFile << this->_upperLeftCol << " ";
    outFile << this->_bottomRightRow << " ";
    outFile << this->_bottomRightCol << " ";
  }

  /// \brief Deserialize a bounding box from a file
  /// \param inFile File to deserialized
  /// \return Bounding box deserialized from the input file
  static BoundingBox deserializeBoundingBox(std::ifstream &inFile) {
    uint32_t
        upperLeftRow = 0,
        upperLeftCol = 0,
        bottomRightRow = 0,
        bottomRightCol = 0;

    inFile >> upperLeftRow;
    inFile >> upperLeftCol;
    inFile >> bottomRightRow;
    inFile >> bottomRightCol;

    return BoundingBox(upperLeftRow,
                       upperLeftCol,
                       bottomRightRow,
                       bottomRightCol);
  }

  /// \brief Assignement operator
  /// \param other Bounding box to copy
  /// \return Reference to the copied Bounding box
  BoundingBox &operator=(const BoundingBox &other) {
    if (this != &other) {
      this->setUpperLeftRow(other.getUpperLeftRow());
      this->setUpperLeftCol(other.getUpperLeftCol());
      this->setBottomRightRow(other.getBottomRightRow());
      this->setBottomRightCol(other.getBottomRightCol());
    }
    return *this;
  }

  /// \brief Bounding box output stream operator
  /// \param os Output stream
  /// \param box Bounding box to print
  /// \return Output stream with the data printed
  friend std::ostream &operator<<(std::ostream &os, const BoundingBox &box) {
    os
        << "Pixel Top Left (" << box._upperLeftRow << "/" << box._upperLeftCol
        << ")"
        << "Pixel Bottom Right (" << box._bottomRightRow << "/"
        << box._bottomRightCol << ")"
        << " Height: " << box.getHeight() << " Width: " << box.getWidth();
    return os;
  }

  /// \brief Equality operator to test purpose
  /// \param bB Bounding box to compare with
  /// \return True is the bounding box are equal, else False
  bool operator==(const BoundingBox &bB) const {
    return
        this->getUpperLeftRow() == bB.getUpperLeftRow()
            && this->getUpperLeftCol() == bB.getUpperLeftCol()
            && this->getBottomRightCol() == bB.getBottomRightCol()
            && this->getBottomRightRow() == bB.getBottomRightRow();
  }

 private:
  uint32_t
      _upperLeftRow,        ///< Bounding box upper left row
      _upperLeftCol,        ///< Bounding box upper left col
      _bottomRightRow,      ///< Bounding box bottom right row
      _bottomRightCol;      ///< Bounding box bottom right col
};
}

#endif //FEATURECOLLECTION_BOUNDINGBOX_H
