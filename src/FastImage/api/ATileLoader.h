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

/// @file ATileLoader.h
/// @author Alexandre Bardakoff - Timothy Blattner
/// @date  7/25/17
/// @brief Main interface to construct a tile loader

#ifndef FASTIMAGE_TILELOADER_H
#define FASTIMAGE_TILELOADER_H

#include <htgs/api/ITask.hpp>
#include <algorithm>
#include <utility>
#include <cstring>
#include "FastImage/data/TileRequestData.h"
#include "FastImage/data/DataType.h"

namespace fi {
/// \namespace fi FastImage namespace

/// \brief Tile Loader interface.
/// \details
/// Can't be instantiate. Take a
/// TileRequestData and produce a
/// htgs::MemoryData<ViewData>>. Has to be inherited to create a new tile
/// loader. The new Tile loader has to override
/// and implement the following functions:
/// \code
///     virtual std::string getName() = 0;
///     virtual ATileLoader *copyTileLoader() = 0;
///     virtual uint32_t getImageHeight(uint32_t level = 0) const = 0;
///     virtual uint32_t getImageWidth(uint32_t level = 0) const = 0;
///     virtual uint32_t getTileWidth(uint32_t level = 0) const = 0;
///     virtual uint32_t getTileHeight(uint32_t level = 0) const = 0;
///     virtual short getBitsPerSample() const = 0;
///     virtual uint32_t getNbPyramidLevels() const = 0;
///     virtual double loadTileFromFile(UserType *tile, uint32_t
///         indexRowGlobalTile, uint32_t indexColGlobalTile) = 0;
/// \endcode
/// and has to set the _bitsPerSample protected attribute.
/// The overloaded copy function should also copy the following:
/// _cache / _filePath / _bitsPerSample / numThreads, which can be taken from
/// the available getters.
/// \tparam UserType Data Type wanted by the user,
/// which is stored within a fi::View
template<typename UserType>
class ATileLoader : public htgs::ITask<fi::TileRequestData<UserType>,
                                       fi::TileRequestData<UserType> > {
 public:
  /// \brief Any Tile loader default constructor
  /// \param filePath File path
  /// \param numThreads number of threads used by the tile loader
  explicit ATileLoader(std::string filePath, size_t numThreads = 1)
      : htgs::ITask<fi::TileRequestData<UserType>,
                    fi::TileRequestData<UserType> >(numThreads),
        _filePath(std::move(filePath)) {}

  /// \brief Default destructor
  ~ATileLoader() = default;

  /// \brief Get File path
  /// \return File path
  const std::string &getFilePath() const { return _filePath; }

  // Function needed by HTGS
  /// \brief Initialize function
  /// \details Associate a cache from the pool to a specific tile loader
  void initialize() final {
    _cache = _allCache[this->getPipelineId()];
  };

  /// \brief Load a tile from a file, populate the current view, and send the
  /// view to the view counter.
  /// \details Processes the requested tile by first checking the cache,
  /// if it is not in the cache, then the
  /// tile is loaded from the disk. The data is copied into a fi::View and sent
  /// to the view counter.
  /// \param tileRequestData the requested tile to load
  void executeTask
      (std::shared_ptr<fi::TileRequestData<UserType>> tileRequestData) final {
    CachedTile<UserType> *cachedTile;
    uint32_t row = tileRequestData->getIndexRowTileAsked();
    uint32_t col = tileRequestData->getIndexColTileAsked();

    // Get locked tile from the cache, can be empty or not
    cachedTile = _cache->getLockedTile(row, col);

    // Set the tile if empty
    if (cachedTile->isNewTile()) {
      cachedTile->setNewTile(false);
      _cache->addTimeDisk(loadTileFromFile(cachedTile->getData(), row, col));
    }

    // Copy the tile or part of it into the view
    copyTileToView(tileRequestData, cachedTile);
    cachedTile->unlock();

    htgs::m_data_t<View<UserType>> viewData = tileRequestData->getViewData();
    this->addResult(tileRequestData);
  }

  /// \brief Copy (part of or all) the cached tile to the view
  /// \param tileRequestData Destination tile request
  /// \param cachedTile Source cached tile
  void copyTileToView(
      std::shared_ptr<fi::TileRequestData<UserType>> tileRequestData,
      CachedTile<UserType> *cachedTile) {
    uint32_t
        rowFrom = tileRequestData->getRowFrom(),
        colFrom = tileRequestData->getColFrom(),
        rowDest = tileRequestData->getRowDest(),
        colDest = tileRequestData->getColDest(),
        heightToCopy = tileRequestData->getHeightToCopy(),
        widthToCopy = tileRequestData->getWidthToCopy(),
        tileWidth = tileRequestData->getTileWidth(),
        viewWidth = tileRequestData->getViewWidth();

    View<UserType> *dest = tileRequestData->getViewData()->get();
    CachedTile<UserType> *src = cachedTile;

    for (uint32_t r = 0; r < heightToCopy; ++r) {
      std::copy_n(
          src->getData() + ((rowFrom + r) * tileWidth + colFrom),
          widthToCopy,
          dest->getData() + ((rowDest + r) * viewWidth + colDest)
      );
    }
  }

  /// \brief Set the caches
  /// \param allCache Caches to set
  void setCache(std::vector<fi::FigCache<UserType> *> &allCache) {
    _allCache = allCache;
  }

  /// \brief ATileLoader copy function used by HTGS to create a new ATileLoader,
  /// will call copyTileLoader more specialize
  /// \return ATileLoader copied
  ATileLoader *copy() final {
    auto tileLoader = copyTileLoader();
    tileLoader->setCache(this->_allCache);
    return tileLoader;
  }

  // Function to overload
  /// \brief Get task name
  /// \return Task name
  std::string getName() override = 0;

  /// \brief Copy Function
  /// \return ATileLoader copied
  virtual ATileLoader *copyTileLoader() = 0;

  /// \brief Getter to image Height
  /// \return Image height
  virtual uint32_t getImageHeight(uint32_t level = 0) const = 0;

  /// \brief Getter to image Width
  /// \return Image Width
  virtual uint32_t getImageWidth(uint32_t level = 0) const = 0;

  /// \brief Getter to tile Width
  /// \return Tile Width
  virtual uint32_t getTileWidth(uint32_t level = 0) const = 0;

  /// \brief Getter to tile Height
  /// \return Tile Height
  virtual uint32_t getTileHeight(uint32_t level = 0) const = 0;

  /// \brief Get file bits per samples
  /// \return File bits per sample
  virtual short getBitsPerSample() const = 0;

  /// \brief Get number of pyramid levels
  /// \return Number of Pyramid levels
  virtual uint32_t getNbPyramidLevels() const = 0;

  /// \brief Get the down scalar factor for a specific pyramid level
  /// \param level Pyramid level
  /// \return The down scalar factor for a specific pyramid level
  virtual float getDownScaleFactor(uint32_t level = 0) { return 1; }

  /// \brief Load a specific Tile at 0 based indexes (indexRowGlobalTile,
  /// indexColGlobalTile) of size
  /// tileHeight x tileWidth
  /// \param tile to load into
  /// \param indexRowGlobalTile Tile row index
  /// \param indexColGlobalTile Tile col index
  /// \return Duration in mS to load a tile from the disk, use for statistics
  /// purpose
  virtual double loadTileFromFile(UserType *tile,
                                  uint32_t indexRowGlobalTile,
                                  uint32_t indexColGlobalTile) = 0;

 protected:
  std::string
      _filePath;          ///< Path to file to load

 private:
  std::vector<fi::FigCache<UserType> *>
      _allCache;          ///< All caches for each pyramid levels

  FigCache<UserType> *
      _cache = nullptr;   ///< Tile Cache
};
}
#endif //FASTIMAGE_TILELOADER_H
