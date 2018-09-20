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


/// @file FigCache.h
/// @author Alexandre Bardakoff - Timothy Blattner
/// @date  8/30/17
/// @brief Cache used by FastImage

#ifndef FASTIMAGE_FIGCACHE_H
#define FASTIMAGE_FIGCACHE_H

#include <queue>
#include <set>
#include <mutex>
#include <sstream>
#include <unordered_map>
#include <list>
#include <cstring>
#include <iostream>
#include <iomanip>

#include "../../FastImage/exception/FastImageException.h"
#include "FastImage/data/CachedTile.h"
#include "../data/DataType.h"

namespace fi {
/// \namespace fi FastImage namespace

/**
  * @class FigCache FigCache.h <FastImage/object/FigCache.h>
  *
  * @brief Fast Image cache.
  *
  * @details Fast Image cache, which is used to limit the image IO. Is uses a
  * LRU policy. The amount of cached tiles is set up at construction.
  * The cached tiles are saved into a matrix which has the same dimension as the
  * image.
  *
  * @tparam UserType Pixel Type asked by the end user
  **/
template<typename UserType>
class FigCache {
  using CachedTileType = CachedTile<UserType> *;
 public:
  /// \brief AnyTileCache constructor
  /// \param nbTilesToCache Number of tiles to cache
  explicit FigCache(uint32_t nbTilesToCache)
      : _timeGet(0.0), _timeRecycle(0.0), _timeDisk(0.0),
        _nbTilesCache(nbTilesToCache), _miss(0), _hit(0),
        _numTilesHeight(0), _numTilesWidth(0) {}

  /// \brief Default destructor
  virtual  ~FigCache() {

    // Clean the pool
    while (!_pool.empty()) {
      auto tile = _pool.front();
      _pool.pop();
      delete tile;
    }

    // Clean the matrix
    for (auto row = _mapCache.begin(); row != _mapCache.end(); ++row) {
      for (auto col = (*row).begin(); col != (*row).end(); ++col) {
        (*col) = nullptr;
      }
    }

    // Clean the LRU and delete every stored tile in the cache system
    for (auto itTile = _lru.begin(); itTile != _lru.end(); ++itTile) {
      delete (*itTile);
    }
  };

  /// \brief Init the cache and allocate the cache's memory with the specified
  /// tile width and height from the files.
  /// \details The total number of tiles allocated by default is equal to
  /// 2*numTilesWidth
  /// \param numTilesHeight Number of tiles in a column
  /// \param numTilesWidth Number of tiles in a row
  /// \param tileHeight Tile's height
  /// \param tileWidth Tile's width
  void initCache(uint32_t numTilesHeight,
                 uint32_t numTilesWidth,
                 uint32_t tileHeight,
                 uint32_t tileWidth) {

    uint32_t nbTilesInImage = numTilesHeight * numTilesWidth;

    _numTilesHeight = numTilesHeight;
    _numTilesWidth = numTilesWidth;

    // If the number of tiles to be cached has been set to 0 (default value),
    // set the number to 2 * number of tiles in a row
    if (_nbTilesCache == 0) { _nbTilesCache = 2 * numTilesWidth; }

    // If the number of tiles to be cached is superior to the number of tiles in
    // the image, set it to the number of tiles in the image
    if (nbTilesInImage < _nbTilesCache) { _nbTilesCache = nbTilesInImage; }

    // Create the matrix
    for (uint32_t row = 0; row < numTilesHeight; ++row) {
      std::vector<CachedTileType> tempV;
      for (uint32_t col = 0; col < numTilesWidth; ++col) {
        tempV.push_back(nullptr);
      }
      _mapCache.push_back(tempV);
    }

    // Fill the pool
    for (uint32_t tileCnt = 0; tileCnt < _nbTilesCache; ++tileCnt) {
      _pool.push(new CachedTile<UserType>(tileWidth, tileHeight));
    }
  };

  /// \brief Test if the row, column tile is in the cache
  /// \param indexRow Tile row index asked
  /// \param indexCol Tile col index asked
  /// \return True if the tile is in the cache, else False
  bool isInCache(uint32_t indexRow, uint32_t indexCol) {
    return !(nullptr == _mapCache[indexRow][indexCol]);
  }

  /// \brief Get a locked tile from the cache system
  /// \details This function is thread safe as the cache will lock
  /// prior to interacting with the cache.
  /// \param indexRow Tile row index asked
  /// \param indexCol Tile col index asked
  /// \return Get a locked tile
  CachedTileType getLockedTile(uint32_t indexRow, uint32_t indexCol) {
    if (!(indexRow >= 0 && indexRow < _numTilesHeight && indexCol >= 0
        && indexCol < _numTilesWidth)) {
      std::stringstream message;
      message << "Tile Loader ERROR: The index is not correct: ("
              << (int) indexRow << ", "
              << (int) indexCol << ")";
      std::string m = message.str();
      throw (FastImageException(m));
    }
    CachedTileType tile;

    this->lock();
    auto begin = std::chrono::high_resolution_clock::now();

    if (isInCache(indexRow, indexCol)) {
      // Tile is in cache
      _hit += 1;
      tile = getCachedLockedTile(indexRow, indexCol);
    } else {
      // Tile is not in the cache
      _miss += 1;
      if (_pool.empty()) { recycleTile(); }
      tile = getNewLockedTile(indexRow, indexCol);
    }
    auto end = std::chrono::high_resolution_clock::now();
    _timeGet += std::chrono::duration_cast<std::chrono::nanoseconds>(
        end - begin).count();
    this->unlock();

    return tile;
  }

  /// \brief Print cache statistics.
  void printStats(double imgeSizeMBytes) {
    std::cout
        << "CacheStats: " << std::endl
        << "    hit = " << _hit
        << " miss = " << _miss
        << " ratio = " << (double) _hit / (double) (_hit + _miss) * 100 << " %"
        << std::endl
        << "    time : Get " << std::scientific << std::setprecision(2)
        << _timeGet
        << "ns Recycle " << std::scientific << std::setprecision(2)
        << _timeRecycle
        << "ns Disk " << std::scientific << std::setprecision(2) << _timeDisk
        << "ns ("
        << (imgeSizeMBytes / _timeDisk) * 10E9 << " MB/s)" << std::endl;
  }

  /// \brief Lock the cache.
  void lock() { _cacheMutex.lock(); }

  /// \brief Unlock the cache.
  void unlock() { _cacheMutex.unlock(); }

  /// Get the number of miss
  /// \return Number of miss
  uint32_t getMiss() const { return _miss; }

  /// Get the number of Hit
  /// \return Number of Hit
  uint32_t getHit() const { return _hit; }

  /// \brief Add a duration to the disk time usage
  /// \param time Duration to add
  void addTimeDisk(double time) { _timeDisk += time; };

  /// \brief Stream output operator for the cache
  /// \param os Output stream
  /// \param cache Cache to print
  /// \return Output stream to put the cache data in
  friend std::ostream &operator<<(std::ostream &os, const FigCache &cache) {
    os << "-------------------------------------------" << std::endl;
    os << "Cache View:" << std::endl;
    os << "Waiting Queue: " << std::endl;
    print_queue(cache._pool);
    os << "MapCache: " << std::endl;
    for (auto row = cache._mapCache.begin(); row != cache._mapCache.end();
         ++row) {
      for (auto col = row->begin(); col != row->end(); ++col) {
        os << *col << " ";
      }
      os << std::endl;
    }
    os << "MapLRU: " << std::endl;
    for (auto elem : cache._mapLRU)
      os << "    " << elem.first << ": "
         << std::distance(cache._lru.begin(), elem.second) << std::endl;

    os << "ListLRU: " << std::endl;
    for (auto elem : cache._lru)
      os << elem << " ";
    os << std::endl;
    os << "timeGet: " << cache._timeGet << " / "
       << "timeRelease: " << cache._timeRecycle << " / "
       << "timeDisk: " << cache._timeDisk << " / "
       << "nbTilesCache: " << cache._nbTilesCache << " / miss: " << cache._miss
       << " / hit: " << cache._hit;
    os << std::endl << "-------------------------------------------"
       << std::endl;
    return os;
  }

  /// \brief Get the total number of Hits and misses from the cache
  /// \return pair<hit, miss>
  std::pair<uint32_t, uint32_t> getHitMissCache() {
    return {this->getHit(), this->getMiss()};
  }

  /// \brief Get the number of tiles allocated in the cache
  /// \return Tiles allocated in the cache
  uint32_t getNbTilesCache() const { return _nbTilesCache; }

  /// \brief Get the pool of available tiles
  /// \return Pool of available tiles
  const std::queue<CachedTileType> &getPool() const { return _pool; }

  /// \brief Get the map cache
  /// \return The Map cache
  const std::vector<std::vector<CachedTileType>> &getMapCache() const {
    return _mapCache;
  }

  /// \brief Get LRU list
  /// \return LRU list
  const std::list<CachedTileType> &getLru() const { return _lru; }

 private:
  /// \brief Private function. Get Least Recently Used Tiles, clean it, and put
  /// it back in the pool.
  void recycleTile() {
    CachedTileType toRecycle;

    auto begin = std::chrono::high_resolution_clock::now();

    // Get LRU Tile
    toRecycle = _lru.back();
    toRecycle->lock();
    _lru.pop_back();

    // Clean The Tile
    _mapLRU.erase(toRecycle);
    _mapCache[toRecycle->getIndexRowGlobal()][toRecycle->getIndexColGlobal()] =
        nullptr;
    toRecycle->setIndexRowGlobal(0);
    toRecycle->setIndexColGlobal(0);
    toRecycle->setNewTile(true);

    // Put it back in the pool
    _pool.push(toRecycle);

    auto end = std::chrono::high_resolution_clock::now();
    _timeRecycle += std::chrono::duration_cast<std::chrono::nanoseconds>(
        end - begin).count();

    toRecycle->unlock();
  }

  /// \brief Private function. Get a new locked tile from the pool.
  /// \param indexRow Tile row index asked
  /// \param indexCol Tile col index asked
  /// \return A locked tile
  CachedTileType getNewLockedTile(uint32_t indexRow, uint32_t indexCol) {
    // Get tile from the pool
    CachedTileType tile = _pool.front();
    tile->lock();
    _pool.pop();

    // Set tile information except data
    tile->setIndexColGlobal(indexCol);
    tile->setIndexRowGlobal(indexRow);

    // Register the tile
    _mapCache[indexRow][indexCol] = tile;
    _lru.push_front(tile);
    _mapLRU[tile] = _lru.begin();
    return tile;
  }

  /// \brief Private function. Get cached locked tile
  /// \param indexRow Tile row index asked
  /// \param indexCol Tile col index asked
  /// \return A locked tile already in the cache
  CachedTileType getCachedLockedTile(uint32_t indexRow, uint32_t indexCol) {
    assert(_mapCache[indexRow][indexCol] != nullptr);

    // Get the tile
    CachedTileType tile = _mapCache[indexRow][indexCol];
    tile->lock();

    // Update the tile position in the LRU
    _lru.erase(_mapLRU[tile]);
    _lru.push_front(tile);
    _mapLRU[tile] = _lru.begin();

    return tile;
  }

  /// \brief Private function. Print a queue, use to print the cache
  /// \param q Queue to print
  static void print_queue(std::queue<CachedTileType> q) {
    while (!q.empty()) {
      std::cout << q.front() << " ";
      q.pop();
    }
    std::cout << std::endl;
  }

  std::queue<CachedTileType>
      _pool;                  ///< Pool of new tile

  std::vector<std::vector<CachedTileType>>
      _mapCache;              ///< Matrix of cached tiles

  std::unordered_map<CachedTileType,
                     typename std::list<CachedTileType>::const_iterator>
      _mapLRU;                ///< Map between the Tile and it's position

  std::list<CachedTileType>
      _lru;                   ///< List to save the Tile order

  std::mutex
      _cacheMutex;            ///< Cache mutex

  double
      _timeGet,               ///< Time to get a tile from the cache
  ///< (use for statistics)
      _timeRecycle,           ///< Time to release a tile from the cache
  ///< (use for statistics)
      _timeDisk;              ///< Disk time use for stats

  uint32_t
      _nbTilesCache,          ///< Number of tiles allocated
      _miss,                  ///< Number of tile miss (tile get from the disk)
      _hit,                   ///< Number of tile hit (tile get from the cache)
      _numTilesHeight,        ///< Number of tiles in a column
      _numTilesWidth;         ///< Number of tiles in a row
};
}
#endif //FASTIMAGE_FIGCACHE_H
