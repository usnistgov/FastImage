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

//
// Created by anb22 on 9/11/17.
//

#ifndef HTGS_FAST_IMAGE_MASKANALYSER_H
#define HTGS_FAST_IMAGE_MASKANALYSER_H

#include <iostream>
#include "FastImage/FeatureCollection/FeatureCollection.h"
#include <include/gtest/gtest.h>

template<typename UserType>
class MaskAnalyser {
 private:
  uint32_t
      _imageHeight,
      _imageWidth;

  std::vector<fc::BoundingBox>
      _boundingBoxes;

  std::vector<uint32_t *>
      _bitMasks;
 public:
  MaskAnalyser(uint32_t imageHeight, uint32_t imageWidth) : _imageHeight(
      imageHeight), _imageWidth(imageWidth) {
    _boundingBoxes.clear();
    _bitMasks.clear();
  }

  virtual ~MaskAnalyser() {
    for (auto bitMask : _bitMasks) {
      delete[] bitMask;
    }
  }

  void initialize(uint64_t nbLabels) {
    for (uint64_t label = 1; label <= nbLabels; ++label) {
      fc::BoundingBox bB(_imageHeight, _imageWidth, 0, 0);
      _boundingBoxes.push_back(bB);
      _bitMasks.emplace_back(nullptr);
    }
  }

  void findBoundingBoxes(const uint64_t *labels) {
    uint64_t label;
    fc::BoundingBox *bB;

    for (uint32_t row = 0; row < _imageHeight; ++row) {
      for (uint32_t col = 0; col < _imageWidth; ++col) {
        label = labels[row * _imageWidth + col];
        if (label != 0) {
          bB = &_boundingBoxes[label - 1];
          if (row < bB->getUpperLeftRow()) { bB->setUpperLeftRow(row); }
          if (col < bB->getUpperLeftCol()) { bB->setUpperLeftCol(col); }
          if (row + 1 > bB->getBottomRightRow()) {
            bB->setBottomRightRow(row + 1);
          }
          if (col + 1 > bB->getBottomRightCol()) {
            bB->setBottomRightCol(col + 1);
          }
        }
      }
    }
  }

  void setBitMask(const uint64_t *labels) {
    uint64_t
        featureIndex = 0,
        featureValue = 0;

    uint32_t
        rowMin = 0,
        rowMax = 0,
        colMin = 0,
        colMax = 0,
        wordPosition = 0,
        bitPosition = 0,
        absolutePosition = 0,
        ulRowL = 0,
        ulColL = 0;

    for (auto bB : this->_boundingBoxes) {
      this->_bitMasks[featureIndex] = new uint32_t[(uint32_t) ceil(
          (bB.getHeight() * bB.getWidth()) / 32.)]();
      featureValue = static_cast<uint64_t>(featureIndex + 1);
      rowMin = bB.getUpperLeftRow();
      rowMax = bB.getUpperLeftRow() + bB.getHeight();
      colMin = bB.getUpperLeftCol();
      colMax = bB.getUpperLeftCol() + bB.getWidth();
      for (uint32_t rowG = rowMin; rowG < rowMax; ++rowG) {
        for (uint32_t colG = colMin; colG < colMax; ++colG) {
          if (labels[rowG * _imageWidth + colG] == featureValue) {
            ulRowL = rowG - bB.getUpperLeftRow();
            ulColL = colG - bB.getUpperLeftCol();
            absolutePosition = ulRowL * bB.getWidth() + ulColL;
            wordPosition = absolutePosition >> 5;
            bitPosition = (uint32_t) abs(
                32 - ((int32_t) absolutePosition
                    - ((int32_t) wordPosition << 5)));
            this->_bitMasks[featureIndex][wordPosition] =
                (this->_bitMasks[featureIndex][wordPosition])
                    | (1 << (bitPosition - 1));
          }
        }
      }
      featureIndex++;
    }
  }

  void save(const std::string &path) {
    fc::FeatureCollection
        fc, fc2;

    fc.setImageHeight(this->_imageHeight);
    fc.setImageWidth(this->_imageWidth);

    for (size_t i = 0; i < this->_bitMasks.size(); ++i) {
      fc.addFeature((uint32_t) i + 1,
                    this->_boundingBoxes[i],
                    this->_bitMasks[i]);
    }
    fc.preProcessing();
    fc.serialize(path);
    fc2.deserialize(path);
    ASSERT_TRUE(fc == fc2);
  }

};

#endif //HTGS_FAST_IMAGE_MASKANALYSER_H
