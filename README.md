# Fast Image (FI) : A High-Performance Accessor for Processing Gigapixel Images

An application programming interface to access big images

The API is designed to aid in creating image processing algorithms to
obtain performance across CPUs and big images.

## Content

- [Installation Instructions](#installation-instructions)
	- [Dependencies](#dependencies)
	- [Building Fast Image](#building-fast-image)
- [Motivation](#motivation)
- [Approach](#approach)
- [Architecture](#architecture)
- [Steps to Programming with Fast Image](#steps-to-programming-with-fast-image)
	- [Linking Fast Image](#linking-_fast-image_)
	- [API overview](#api-overview)
	- [How to create a Tile Loader ? How to access a specific file ?](#how-to-create-a-tile-loader-how-to-access-a-specific-file-)
	- [Getting started](#getting-started)
		- [Image traversal](#image-traversal)
		- [Spatial convolution](#spatial-convolution)
		- [OpenCV convolution](#opencv-convolution)
		- [Feature Collection usage](#feature-collection-usage)
- [Credits](#credits)
- [Contact Us](#contact-us)

# Installation Instructions

## Dependencies

1) g++/gcc version 4.8.4+

2) HTGS (https://github.com/usnistgov/htgs)

3) LibTIFF (http://www.simplesystems.org/libtiff/)

4) doxygen (www.doxygen.org/) [optional / Documentation]

## Building Fast Image
**CMake Options:**

CMAKE_INSTALL_PREFIX - Where to install Fast Image (and documentation)

BUILD_DOXYGEN - Creates doxygen documentation

RUN_GTEST - Compiles and runs google unit tests for Fast Image ('make run-test' to re-run)

```
 :$ cd <FastImage_Directory>
 :<FastImage_Directory>$ mkdir build && cd build
 :<FastImage_Directory>/build$ ccmake ../ (or cmake-gui)

 'Configure' and setup cmake parameters
 'Configure' and 'Build'

 :<FastImage_Directory>/build$ make
 :<FastImage_Directory>/build$ [sudo] make install
```

# Motivation

The hardware landscape for high-performance computing currently
features compute nodes with a high degree of parallelism within a node
(e.g., 46 cores for the newly-announced Qualcomm Centriq CPU, 32
physical cores for the AMD Epyc CPU, and 24 logical cores for an Intel
Xeon Skylake CPU), that increases with every new hardware generation.
By contrast, the amount of memory available per core is not increasing
in a commensurate manner and may even be decreasing especially when
considered on a per-core basis.  Furthermore, while the computational
capacity of these systems keeps on improving, their programmability
remains quite challenging.  As such, designing image processing
algorithms to minimize memory usage is a key strategy to taking
advantage of this parallelism and support concurrent users or the
multi-threaded processing to large images.

# Approach

_Fast Image_ improves programmer productivity by providing high-level
abstractions, _View_ and _Tile_, along with routines that build on
these abstractions to operate across an entire image without actually
loading it in memory.  The library operates on tiles (e.g., a
1024x1024 partial image out of a very large 100K x 100K image) with
possibly a halo of pixels around an individual tile.  _Fast Image_
only loads a small number of tiles to maintain a low-memory footprint
and manages an in-memory cache.  Furthermore, the library takes
advantage of multi-core computing by offloading tiles to compute
threads as soon as they become available.  This allows for multiple
users to maintain high throughput, while processing several images or
views concurrently.

# Architecture

_Fast Image_ architecture shows how the system work and interact with
the algorithm.  First of all it works asynchronously from the
algorithm.  Secondly each part of Fast Image will be on on different
threads.  Finally the static memory manager guaranty that the amount
of memory will be limited as asked.
 
When an algorithm will ask _n_ views through _View Request_.  _Fast
Image_ will use this _View Request_ to build a _view_ and make it
available as soon as possible.  The algorithm will be able to use it
(and release it).  In the mean time, if enough memory is available an
other _view_ will be created.

The _View_ creation go through 3 steps:
1. _View Loader_: Request memory from the memory manager and split the
_View Request_, to _n_ _Tile Loader_ and send them to the _Tile
Loader_.
2. _Tile Loader_: Specific to the file _Fast Image_ access to.  Will
ask the _Tile_ to _Tile Cache_, if it not available the _Tile_ will be
loaded from the file, then cast and copy to the _Tile Cache_.  From the
cache only the interesting _Tile_'s part will be copied to the _View_.
3. _View Counter_: Wait to the _view_ to be fully loaded from file's
parts.  Then build the ghost region if needed, and send the complete
_view_ to the algorithm.

Following the Fast Image Graph and it's interaction with the algorithm:

![Fast Image Graph](images/FIGraph.png "Fast Image Graph and Algorithm interaction")

# Steps to Programming with Fast Image

## Linking _Fast Image_

_Fast Image_ can be easily link to any C++ 11 compliant code using
cmake.  Add the path to the folder 
FastImageDirectory/cmake-modules/ to the CMAKE_MODULE_PATH variable in your CMakeLists.txt.
Then add the following lines in your CMakeLists.txt: 
```cmake
find_package(FastImage REQUIRED)
include_directories(${HTGS_INCLUDE_DIR})
include_directories(${FastImage_INCLUDE_DIR})
link_libraries(${FastImage_LIBRARIES})
```

## API overview

3 API exists in _Fast Image_:
1. The FastImage object to access views of an image
2. The _View_ object to access pixel/data in the _View_
3. FeatureCollection to get the information on region of interests 
4. Tile Loader

## How to create a Tile Loader ? How to access a specific file ?

To access to a new file format, a specific _Tile Loader_ is needed.  A
specific _Tile Loader_ class will inherit from the class
_ATileLoader_.

The following methods need to be implemented: 
```cpp
  // Constructor
  ATileLoader                       (const std::string &filePath, size_t numThreads = 1);

  // Copy function to duplicate the Tile Loader into n threads 
  ATileLoader*  copyTileLoader      ();
  
  // Task Name getter used for graph generation 
  std::string   getName             ();
  
  // Basic file information getter 
  uint32_t      getImageHeight      (uint32_t level = 0) const;
  uint32_t      getImageWidth       (uint32_t level = 0) const;
  uint32_t      getTileWidth        (uint32_t level = 0) const;
  uint32_t      getTileHeight       (uint32_t level = 0) const;
  short         getBitsPerSample    () const;
  uint32_t      getNbPyramidLevels  () const;
  
  // Load a specific tile from the file, the tile has already allocated. Return the disk loading time in ms.
  double        loadTileFromFile    (UserType *tile, uint32_t indexRowGlobalTile, uint32_t indexColGlobalTile);
```

Here a Tile Loader for Greyscale Tiled Tiff:

```cpp
explicit GrayscaleTiffTileLoader(const std::string &fileName,
                                   size_t numThreads = 1)
      : ATileLoader<UserType>(fileName,
                              numThreads) {
    short samplesPerPixel = 0;

    // Open the file
    this->_tiff = TIFFOpen(fileName.c_str(), "r");
    if (this->_tiff != nullptr) {
      if (TIFFIsTiled(_tiff) == 0) {
        std::stringstream message;
        message << "Tile Loader ERROR: The image is not tiled.";
        std::string m = message.str();
        throw (FastImageException(m));
      }
      // Load/parse header
      TIFFGetField(this->_tiff, TIFFTAG_IMAGEWIDTH, &(this->_imageWidth));
      TIFFGetField(this->_tiff, TIFFTAG_IMAGELENGTH, &(this->_imageHeight));
      TIFFGetField(this->_tiff, TIFFTAG_TILEWIDTH, &this->_tileWidth);
      TIFFGetField(this->_tiff, TIFFTAG_TILELENGTH, &this->_tileHeight);
      TIFFGetField(this->_tiff, TIFFTAG_SAMPLESPERPIXEL, &samplesPerPixel);
      TIFFGetField(this->_tiff, TIFFTAG_BITSPERSAMPLE, &(this->_bitsPerSample));
      TIFFGetField(this->_tiff, TIFFTAG_SAMPLEFORMAT, &(this->_sampleFormat));

      // Test if the file is greyscale
      if (samplesPerPixel != 1) {
        std::stringstream message;
        message
            << "Tile Loader ERROR: The image is not greyscale: "
               "SamplesPerPixel = " << samplesPerPixel << ".";
        std::string m = message.str();
        throw (FastImageException(m));
      }

      // Interpret undefined data format as unsigned integer data
      if (_sampleFormat < 1 && _sampleFormat > 3) {
        _sampleFormat = 1;
      }

    } else {
      std::stringstream message;
      message << "Tile Loader ERROR: The image can not be opened.";
      std::string m = message.str();
      throw (FastImageException(m));
    }
  }

  /// \brief TiffTileLoader default destructor
  /// \details Destroy a GrayscaleTiffTileLoader and close the underlined file
  ~GrayscaleTiffTileLoader() {
    if (this->_tiff != nullptr) {
      TIFFClose(this->_tiff);
    }
  }

  /// \brief Get Image height
  /// \return Image height in px
  uint32_t getImageHeight(uint32_t level = 0) const override {
    return _imageHeight;
  }

  /// \brief Get Image width
  /// \return Image width in px
  uint32_t getImageWidth(uint32_t level = 0) const override {
    return _imageWidth;
  }

  /// \brief Get tile width
  /// \return Tile width in px
  uint32_t getTileWidth(uint32_t level = 0) const override {
    return _tileWidth;
  }

  /// \brief Get tile height
  /// \return Tile height in px
  uint32_t getTileHeight(uint32_t level = 0) const override {
    return _tileHeight;
  }

  /// \brief Get the bits per sample from the file
  /// \details Return the number of bits per sample as defined in the libtiff library:
  /// https://www.awaresystems.be/imaging/tiff/tifftags/bitspersample.html
  /// \return the number of bits per sample
  short getBitsPerSample() const override {
    return _bitsPerSample;
  }

  /// \brief Getter to the number of pyramids levels
  /// \details Get the number of pyramids levels, in this case 1, because the
  /// assumption is the image is planar
  /// \return
  uint32_t getNbPyramidLevels() const override { return 1; }

  /// \brief Loader to the tile with a specific cast
  /// \brief Load a specific tile from the file into a tile buffer,
  /// casting each pixels from  FileType to UserType
  /// \tparam FileType Type deduced from the file
  /// \param src Source data buffer
  /// \param dest Tile buffer
  template<typename FileType>
  void loadTile(tdata_t src, UserType *dest) {
    for (uint32_t row = 0; row < _tileHeight; ++row) {
      for (uint32_t col = 0; col < _tileWidth; ++col) {
        dest[row * _tileWidth + col] =
            (UserType) ((FileType *) (src))[row * _tileWidth + col];
      }
    }
  }

  /// \brief Get down scale Factor for pyramid images. The tiff image for this
  /// loader are not pyramids, so return 1
  /// \param level level to ask--> Not used
  /// \return 1
  float getDownScaleFactor(uint32_t level = 0) override { return 1; }

  /// \brief Load a tile from the disk
  /// \details Allocate a buffer, load a tile from the file into the buffer,
  /// cast each pixel to the right format into parameter tile, and close
  /// the buffer.
  /// \param tile Pointer to a tile already allocated to fill
  /// \param indexRowGlobalTile Row index tile asked
  /// \param indexColGlobalTile Column Index tile asked
  /// \return Duration in mS to load a tile from the disk, use for statistics
  /// purpose
  double loadTileFromFile(UserType *tile,
                          uint32_t indexRowGlobalTile,
                          uint32_t indexColGlobalTile) override {
    tdata_t tiffTile = nullptr;
    tiffTile = _TIFFmalloc(TIFFTileSize(_tiff));
    auto begin = std::chrono::high_resolution_clock::now();
    TIFFReadTile(_tiff,
                 tiffTile,
                 indexColGlobalTile * _tileWidth,
                 indexRowGlobalTile * _tileHeight,
                 0,
                 0);
    auto end = std::chrono::high_resolution_clock::now();
    double diskDuration = (std::chrono::duration_cast<std::chrono::nanoseconds>(
        end - begin).count());
    std::stringstream message;
    switch (this->_sampleFormat) {
      case 1 :
        switch (this->_bitsPerSample) {
          case 8:loadTile<uint8_t>(tiffTile, tile);
            break;
          case 16:loadTile<uint16_t>(tiffTile, tile);
            break;
          case 32:loadTile<uint32_t>(tiffTile, tile);
            break;
          case 64:loadTile<uint64_t>(tiffTile, tile);
            break;
          default:
            message
                << "Tile Loader ERROR: The data format is not supported for "
                   "unsigned integer, number bits per pixel = "
                << this->_bitsPerSample;
            std::string m = message.str();
            throw (FastImageException(m));
        }
        break;
      case 2:
        switch (this->_bitsPerSample) {
          case 8:loadTile<int8_t>(tiffTile, tile);
            break;
          case 16:loadTile<int16_t>(tiffTile, tile);
            break;
          case 32:loadTile<int32_t>(tiffTile, tile);
            break;
          case 64:loadTile<int64_t>(tiffTile, tile);
            break;
          default:
            message
                << "Tile Loader ERROR: The data format is not supported for "
                   "signed integer, number bits per pixel = "
                << this->_bitsPerSample;
            std::string m = message.str();
            throw (FastImageException(m));
        }
        break;
      case 3:
        switch (this->_bitsPerSample) {
          case 8:loadTile<float>(tiffTile, tile);
            break;
          case 16:loadTile<float>(tiffTile, tile);
            break;
          case 32:loadTile<float>(tiffTile, tile);
            break;
          case 64:loadTile<double>(tiffTile, tile);
            break;
          default:
            message
                << "Tile Loader ERROR: The data format is not supported for "
                   "float, number bits per pixel = " << this->_bitsPerSample;
            std::string m = message.str();
            throw (FastImageException(m));
        }
        break;
      default:
        message
            << "Tile Loader ERROR: The data format is not supported, sample "
               "format = " << this->_sampleFormat;
        std::string m = message.str();
        throw (FastImageException(m));
    }

    _TIFFfree(tiffTile);
    return diskDuration;
  }

  /// \brief Copy function used by HTGS to use multiple Tile Loader
  /// \return  A new ATileLoader copied
  ATileLoader<UserType> *copyTileLoader() override {
    return new GrayscaleTiffTileLoader<UserType>(this->getNumThreads(),
                                                 this->getFilePath(),
                                                 *this);
  }

  /// \brief Get the name of the tile loader
  /// \return Name of the tile loader
  std::string getName() override {
    return "TIFF Tile Loader";
  }

 private:
  /// \brief TiffTileLoader constructor used by the copy operator
  /// \param numThreads Number of thread used by the tiff tile loader
  /// \param filePath File path
  /// \param from Origin TiffTileLoader
  GrayscaleTiffTileLoader(size_t numThreads,
                          const std::string &filePath,
                          const GrayscaleTiffTileLoader &from)
      : ATileLoader<UserType>(filePath, numThreads) {
    this->_tiff = TIFFOpen(filePath.c_str(), "r");
    this->_imageWidth = from._imageWidth;
    this->_imageHeight = from._imageHeight;
    this->_tileWidth = from._tileWidth;
    this->_tileHeight = from._tileHeight;
    this->_bitsPerSample = from._bitsPerSample;
    this->_sampleFormat = from._sampleFormat;
  }

  TIFF *
      _tiff = nullptr;             ///< Tiff file pointer

  uint32_t
      _imageHeight = 0,           ///< Image height in pixel
      _imageWidth = 0,            ///< Image width in pixel
      _tileHeight = 0,            ///< Tile height
      _tileWidth = 0;             ///< Tile width

  short
      _sampleFormat = 0,          ///< Sample format as defined by libtiff
      _bitsPerSample = 0;         ///< Bit Per Sample as defined by libtiff
};
```

## Getting started
```diff
The code in this section is only PSEUDOCODE, and is not meant to be executed as is.
```

### Image traversal

Here a little program to go through all the pixel in a tiled greyscale tiff image:
```cpp
// Tile loader creation depending on the image
fi::ATileLoader<uint32_t> *tileLoader = new fi::GrayscaleTiffTileLoader<uint32_t>(pathImage); 

// Fast Image accessor creation
auto *fi = new fi::FastImage<uint32_t>(tileLoader, 0);

// Configure and run the Fast Image
fi->configureAndRun();

// Requesting all tiles 
fi->requestAllTiles(true);

// Getting Data while Fast Image is processing some
while (fi->isGraphProcessingTiles()) {

    // Getting an image's view 
    auto pView = fi->getAvailableViewBlocking();
    
    // If not the last one
    if (pView != nullptr) {
    
        // Resolve shared_ptr
        auto view = pView->get();
        // Get information on the tile and looping through it 
        for (int32_t r = 0; r < view->getTileHeight(); ++r) {
            for (int32_t c = 0; c < view->getTileWidth(); ++c) {
                // Getting the pixel
                view->getPixel(r, c);
            }
        }
        
        // Release the memory
        pView->releaseMemory();
    }
}

// Waiting for FI to copmlete all it tasks
fi->waitForGraphComplete();

// Delete Fast Image (it will take care of the Tile Loader)
delete fi;

```

### Spatial convolution

This code do a spatial convolution on a tiled greyscale tiff image (kernel creation and tile writing are not showed). A full example is given in the fast image example. 
 ```cpp
int radius = 6;
float pixelOut = 0;
double sigma = 2;
int32_t sizeKernel = radius * 2 + 1;


fi::ATileLoader<uint32_t> *tileLoader = new fi::GrayscaleTiffTileLoader<uint32_t>(pathImage); // Tile loader creation depending on the image
auto *fi = new fi::FastImage<uint32_t>(tileLoader, 0); // Fast Image accessor creation

std::vector<double> kernel = gaussian(radiusKernel, sigma); // Kernel creation 
auto *tileOut = new float[fi->getTileWidth() * fi->getTileHeight()]();

fi->configureAndRun(); // Configure and run the Fast Image
fi->requestAllTiles(true); // Requesting all tiles 

// Getting Data while Fast Image is processing some
while (fi->isGraphProcessingTiles()) {
    auto pView = fi->getAvailableViewBlocking(); // Getting an image's view 
    // If not the last one
    if (pView != nullptr) {
    
        auto view = pView->get(); // Resolve shared_ptr
        // Get information on the tile and looping through it 
        for (int32_t r = 0; r < view->getTileHeight(); ++r) {
            for (int32_t c = 0; c < view->getTileWidth(); ++c) {
                pixelOut = 0;
                for (int32_t rK = -radius; rK <= radius; ++rK) {
                    for (int32_t cK = -radius; cK <= radius; ++cK) {
                        pixelOut += view->getPixel(r + rK, c + cK) * kernel[(rK + radius) * sizeKernel + (cK + radius)];
                    }
                }
                tileOut[r * tileWidth + c] = pixelOut / (view->getTileHeight() * view->getTileWidth());
            }
        }
        	
        writeTile(tileOut, view->getGlobalXOffset(), view->getGlobalYOffset());
        // Release the memory
        pView->releaseMemory();
    }
}

delete [] tileOut;
delete (fi)
```

### OpenCV convolution
 
This code do a convolution with OpenCV on a tiled greyscale tiff image (kernel creation and tile writing are not showed). A full example is given in the fast image example. 
 ```cpp
using namespace cv; 

int radius = 6;
float pixelOut = 0;
double sigma = 2;
int32_t sizeKernel = radius * 2 + 1;


fi::ATileLoader<uint32_t> *tileLoader = new fi::GrayscaleTiffTileLoader<uint32_t>(pathImage); // Tile loader creation depending on the image
auto *fi = new fi::FastImage<uint32_t>(tileLoader, 0); // Fast Image accessor creation

Mat kernel = gaussian(radius, sigma, CV_32F); // Kernel creation 

auto *tileOut = new float[fi->getTileWidth() * fi->getTileHeight()]();

fi->configureAndRun(); // Configure and run the Fast Image
fi->requestAllTiles(true); // Requesting all tiles 

auto viewHeight = fi->getViewHeight();
auto viewWidth = fi->getViewWidth();

// Getting Data while Fast Image is processing some
while (fi->isGraphProcessingTiles()) {
    auto pView = fi->getAvailableViewBlocking(); // Getting an image's view 
    // If not the last one
    if (pView != nullptr) {
        auto view = pView->get(); // Resolve shared_ptr
        
        Mat matInput(viewHeight, viewWidth, CV_32F, view->getData())
        Mat tileResult;

        //Do the convolution
        filter2D(matInput, tileResult, CV_32F, _kernel, cvPoint(-1, -1));
        
        cv::Rect roi(radius, radius, view->getTileWidth(), view->getTileHeight());
        cv::Mat<float> matOut = tileResult(roi).clone();
        
        tileOut = (float *)matOut.data;
       
        writeTile(tileOut, view->getGlobalXOffset(), view->getGlobalYOffset());
        // Release the memory
        pView->releaseMemory();
    }
}

delete [] tileOut;
delete (fi)
```

### Feature Collection usage

This code go through a tiled greyscale tiff image (kernel creation and tile writing are not showed) through is mask. A full example is given in the fast image example. 
```cpp
fi::ATileLoader<uint32_t> *tileLoader = new fi::GrayscaleTiffTileLoader<uint32_t>(pathImage); // Tile loader creation depending on the image
auto *fi = new fi::FastImage<uint32_t>(tileLoader, 0); // Fast Image accessor creation

// Feature Collection creation 
fc::FeatureCollection featureCollection;
featureCollection.deserialize(pathFeatureCollection);

fi->configureAndRun();

// Go through all features
for (const auto &feature : featureCollection) {
    // Request views from a feature
    fi->requestFeature(feature);
    while (!fi->isFeatureDone()) {
        auto sh_view = fi->getAvailableViewBlocking();
        auto view = sh_view->get();
        if (view != nullptr) {
            for (auto row = 0; row < view->getTileHeight(); ++row) {
                for (auto col = 0; col < view->getTileWidth(); ++col) {
                    
                    // Test is a pixel is in the bitmask
                    if (feature.isInBitMask(row + view->getGlobalYOffset(), col + view->getGlobalXOffset())) {
                        view->getPixel(row, col);
                    }
                    
                }
            }
            sh_view->releaseMemory();
        }
    }
}
fi->finishedRequestingTiles();
delete fi;
```

# Credits

Alexandre Bardakoff

Timothy Blattner

Walid Keyrouz

Mary Brady

# Contact Us

<a target="_blank" href="mailto:alexandre.bardakoff@nist.gov">Alexandre Bardakoff (alexandre.bardakoff ( at ) nist.gov</a>

<a target="_blank" href="mailto:timothy.blattner@nist.gov">Timothy Blattner (timothy.blattner ( at ) nist.gov</a>
