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

/// @file ViewAnalyser.h
/// @author Alexandre Bardakoff - Timothy Blattner
/// @date  4/6/17
/// @brief Task wich analyse a view to find the different blob into it.

#ifndef FEATURECOLLECTION_VIEWANALYSER_H
#define FEATURECOLLECTION_VIEWANALYSER_H

#include <cstdint>
#include <unordered_set>
#include "FastImage/FeatureCollection/Data/Blob.h"
#include "../Data/ViewAnalyse.h"
namespace fc {
/// \namespace fc FeatureCollection namespace

/**
  * @class ViewAnalyser ViewAnalyser.h <FastImage/FeatureCollection/Tasks/ViewAnalyser.h>
  *
  * @brief View Analyser, HTGS task, take a FastImage view and produce a
  * ViewAnalyse to the BlobMerger.
  *
  * @details HTGS tasks, which run a flood algorithm
  * (https://en.wikipedia.org/wiki/Flood_fill) in a view to find the different
  * connected pixels called blob. Two connected rules are proposed:
  * _4 (North, South, East, West)
  * _8 (North, North-East, North-West, South, South-East, South-West, East, West)
  *
  * @tparam UserType File pixel type
  **/
template<class UserType>
class ViewAnalyser : public htgs::ITask<htgs::MemoryData<fi::View<UserType>>,
                                        ViewAnalyse> {
 public:
  /// \brief View analyser constructor, create a view analyser from the mask
  /// information
  /// \param numThreads Number of threads the view analyser will be executed in 
  /// parallel
  /// \param fi Mask FI
  /// \param rank Rank to the connectivity: 4=> 4-connectivity, 8=> 
  /// 8-connectivity
  /// \param background Background value
  ViewAnalyser(size_t numThreads,
               fi::FastImage<UserType> *fi,
               const uint8_t &rank,
               const UserType &background)
      : ITask<htgs::MemoryData<fi::View<UserType>>, ViewAnalyse>(numThreads),
        _background(background),
        _fi(fi),
        _imageHeight(fi->getImageHeight()),
        _imageWidth(fi->getImageWidth()),
        _rank(rank),
        _vAnalyse(nullptr) {}

  /// \brief Test if a pixel need to be visited
  /// \param row Pixel's row
  /// \param col Pixel's col
  /// \return True if it need to be visited, else False
  inline bool needVisit(int32_t row, int32_t col) {
    return _view->getPixel(row, col) != _background;
  }

  /// \brief Analyse the neighbour of a pixel for a 4-connectivity
  /// \param row Pixel's row
  /// \param col Pixel's col
  void analyseNeighbour4(int32_t row, int32_t col) {
    // Top Pixel
    if (row >= 1) {
      if (needVisit(row - 1, col)) {
        _toVisit.emplace(row - 1, col);
      }
    }
    // Bottom pixel
    if (row + 1 < _tileHeight) {
      if (needVisit((row + 1), col)) {
        _toVisit.emplace(row + 1, col);
      }
    }
    // Left pixel
    if (col >= 1) {
      if (needVisit(row, col - 1)) {
        _toVisit.emplace(row, col - 1);
      }
    }
    // Right pixel
    if (col + 1 < _tileWidth) {
      if (needVisit(row, col + 1)) {
        _toVisit.emplace(row, col + 1);
      }
    }
    // Add a pixel to to merge if the bottom pixel is outside of the tile, but 
    // had a foreground value
    if (row + 1 == _tileHeight
        && row + _view->getGlobalYOffset() + 1 != _imageHeight) {
      if (_view->getPixel(row + 1, col) != _background) {
        _vAnalyse->addToMerge(_currentBlob,
                              std::pair<int32_t, int32_t>(
                                  row + 1 + _view->getGlobalYOffset(),
                                  col + _view->getGlobalXOffset()));
      }
    }
    // Add a pixel to to merge if the right pixel is outside of the tile, but 
    // had a foreground value
    if (col + 1 == _tileWidth
        && col + _view->getGlobalXOffset() + 1 != _imageWidth) {
      if (_view->getPixel(row, col + 1) != _background) {
        _vAnalyse->addToMerge(_currentBlob,
                              std::pair<int32_t, int32_t>(
                                  row + _view->getGlobalYOffset(),
                                  col + 1 + _view->getGlobalXOffset()));
      }
    }

  }

  /// \brief Analyse the neighbour of a pixel for a 8-connectivity
  /// \param row Pixel's row
  /// \param col Pixel's col
  void analyseNeighbour8(int32_t row, int32_t col) {
    int32_t
        minRow = std::max(0, row - 1),
        maxRow = std::min(_view->getTileHeight(), row + 2),
        minCol = std::max(0, col - 1),
        maxCol = std::min(_view->getTileWidth(), col + 2);

    // Analyse the pixels around the pixel
    for (int32_t rowP = minRow; rowP < maxRow; ++rowP) {
      for (int32_t colP = minCol; colP < maxCol; ++colP) {
        if (needVisit(rowP, colP)) {
          _toVisit.emplace(rowP, colP);
        }
      }
    }

    // Add a pixel to to merge if the bottom pixel is outside of the tile,
    // but had a foreground value
    if (row + 1 == _tileHeight
        && row + _view->getGlobalYOffset() + 1 != _imageHeight) {
      if (_view->getPixel(row + 1, col) != _background) {
        _vAnalyse->addToMerge(_currentBlob,
                              std::pair<int32_t, int32_t>(
                                  row + 1 + _view->getGlobalYOffset(),
                                  col + _view->getGlobalXOffset())
        );
      }
    }

    // Add a pixel to to merge if the right pixel is outside of the tile, but 
    // had a foreground value
    if (col + 1 == _tileWidth
        && col + _view->getGlobalXOffset() + 1 != _imageWidth) {
      if (_view->getPixel(row, col + 1) != _background) {
        _vAnalyse->addToMerge(_currentBlob,
                              std::pair<int32_t, int32_t>(
                                  row + _view->getGlobalYOffset(),
                                  col + 1 + _view->getGlobalXOffset()));
      }
    }

    // Add a pixel to to merge if the bottom-right pixel is outside of the tile, 
    // but had a foreground value
    if ((col == _tileWidth - 1 || row == _tileHeight - 1) &&
        row + _view->getGlobalYOffset() + 1 != _imageHeight
        && col + _view->getGlobalXOffset() + 1 != _imageWidth) {
      if (_view->getPixel(row + 1, col + 1) != _background) {
        _vAnalyse->addToMerge(_currentBlob,
                              std::pair<int32_t, int32_t>(
                                  row + 1 + _view->getGlobalYOffset(),
                                  col + 1 + _view->getGlobalXOffset()));
      }
    }

    // Add a pixel to to merge if the top-right pixel is outside of the tile, 
    // but had a foreground value
    if ((row == 0 || col == _tileWidth - 1)
        && row + _view->getGlobalYOffset() > 0
        && col + _view->getGlobalXOffset() + 1 != _imageWidth) {
      if (_view->getPixel(row - 1, col + 1) != _background) {
        _vAnalyse->addToMerge(_currentBlob,
                              std::pair<int32_t, int32_t>(
                                  row - 1 + _view->getGlobalYOffset(),
                                  col + 1 + _view->getGlobalXOffset()));
      }
    }
  }



  /// \brief Execute the task, do a view analyse
  /// \param view View given by the FI
  void executeTask(std::shared_ptr<MemoryData<fi::View<UserType>>> view)
  override {
    _view = view->get();
    //Set tup the environment for the tile
    _tileHeight = _view->getTileHeight();
    _tileWidth = _view->getTileWidth();
    _vAnalyse = new ViewAnalyse();
    _toVisit.clear();
    _currentBlob = nullptr;

    // For every pixel
    for (int32_t row = 0; row < _tileHeight; ++row) {
      for (int32_t col = 0; col < _tileWidth; ++col) {

        // Analyse the neighbour if need be
        while (!_toVisit.empty()) {
          auto neighbourCoord = *_toVisit.begin();
          _toVisit.erase(_toVisit.begin());
          _view->setPixel(neighbourCoord.first,
                          neighbourCoord.second,
                          _background);
          _currentBlob->addPixel(
              _view->getGlobalYOffset() + neighbourCoord.first,
              _view->getGlobalXOffset() + neighbourCoord.second);


          if (_rank == 4) {
            analyseNeighbour4(neighbourCoord.first, neighbourCoord.second);
          } else {
            analyseNeighbour8(neighbourCoord.first, neighbourCoord.second);
          }
        }

        // Save the current blob if need be
        if (_currentBlob != nullptr) {
          _vAnalyse->insertBlob(_currentBlob);
          _currentBlob = nullptr;
        }

        // Test if the current pixel need to be visited
        if (needVisit(row, col)) {
          _currentBlob = new Blob();

          //Create a blob for it
          _currentBlob->addPixel(_view->getGlobalYOffset() + row,
                                 _view->getGlobalXOffset() + col);
          _view->setPixel(row, col, _background);
          // Analyse is neighbour
          if (_rank == 4) {
            analyseNeighbour4(row, col);
          } else {
            analyseNeighbour8(row, col);
          }
        }
      }
    }
    // Save the current blob if the BR pixel  is alone and is a blob by itself
    if (_currentBlob != nullptr) {
      _vAnalyse->insertBlob(_currentBlob);
      _currentBlob = nullptr;
    }

    // Release the view memory
    view->releaseMemory();
    // Add the analyse
    this->addResult(_vAnalyse);
  }

  /// \brief View analyser copy function
  /// \return A new View analyser
  ViewAnalyser<UserType> *copy() override {
    return new ViewAnalyser<UserType>(this->getNumThreads(),
                                      _fi,
                                      _rank,
                                      _background);
  }

  /// \brief Get task name
  /// \return Task name
  std::string getName() override {
    return "View analyser";
  }

 private:
  fi::View<UserType> *
      _view{};                      ///< Current view

  UserType
      _background{};                ///< Pixel background value

  fi::FastImage<UserType> *
      _fi{};                        ///< FI mask

  uint32_t
      _imageHeight{},               ///< Mask height
      _imageWidth{};                ///< Mask width

  uint8_t
      _rank{};                      ///< Rank to the connectivity:
                                    ///< 4=> 4-connectivity, 8=> 8-connectivity

  int32_t
      _tileHeight{},                ///< Tile actual height
      _tileWidth{};                 ///< Tile actual width

  std::set<Coordinate>
      _toVisit{};                   ///< Set of coordinates to visit (neighbour)

  ViewAnalyse
      *_vAnalyse = nullptr;         ///< Current view analyse

  Blob
      *_currentBlob = nullptr;      ///< Current blob
};
}
#endif //FEATURECOLLECTION_VIEWANALYSER_H
