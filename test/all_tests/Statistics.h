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
// Created by anb22 on 11/9/17.
//

#ifndef HTGS_FAST_IMAGE_STATISTICS_H
#define HTGS_FAST_IMAGE_STATISTICS_H

#include <cassert>
#include <cstdint>
#include <vector>
#include <ostream>
#include <numeric>

class Statistics {
  struct Statistic {
    long double
        _mean,
        _stdv,
        _var;

    long long
        _count;

    Statistic(long double mean = 0, long double stdv = 0, long long count = 0)
        : _mean(mean), _stdv(stdv), _count(count), _var(stdv * stdv) {};

    Statistic &operator=(const Statistic &src) {
      this->_mean = src._mean;
      this->_stdv = src._stdv;
      this->_var = src._var;
      this->_count = src._count;
      return *this;
    }

    friend std::ostream &operator<<(std::ostream &os,
                                    const Statistic &statistic) {
      os << "Count: " << statistic._count
         << " Mean: " << statistic._mean
         << " Stdv: " << statistic._stdv
         << " Var: " << statistic._var
         << std::endl;
      return os;
    }
  };

 private:
  std::vector<Statistic> _values;

 public:
  Statistics() {
    _values.clear();
  }

  virtual ~Statistics() { _values.clear(); }

  void addStatistic(long double mean,
                    long double stdv,
                    long long count) {
    _values.emplace_back(mean,
                         stdv,
                         count);
  }

  long double getMean(unsigned long pos) {
    assert(pos < this->_values.size());
    return _values[pos]._mean;
  }

  long double getStdv(unsigned long pos) {
    assert(pos < this->_values.size());
    return _values[pos]._stdv;
  }

  long long getCount(unsigned long pos) {
    assert(pos < this->_values.size());
    return _values[pos]._count;
  }

  long double getGlobalMean() {

    long double mean = 0;
    long long count = 0;

    if (!_values.empty()) {
      mean = _values.begin()->_mean;
      count = _values.begin()->_count;
      for (auto stat = _values.begin() + 1; stat != _values.end(); ++stat) {
        mean = (mean * count + stat->_mean * stat->_count)
            / (count + stat->_count);
        count += stat->_count;
      }
    }
    return mean;
  }

  long double getGlobalStdv() {
    Statistic
        oldS,
        newS;

    if (_values.empty()) { return 0; }

    oldS = *_values.begin();

    for (auto stat = _values.begin() + 1; stat != _values.end(); ++stat) {
      newS._count = oldS._count + stat->_count;
      newS._mean = (oldS._mean * oldS._count + stat->_mean * stat->_count)
          / (newS._count);
      newS._stdv = std::sqrt(
          (oldS._count * (oldS._var)
              + stat->_count * (stat->_var)
              + oldS._count * (oldS._mean - newS._mean)
                  * (oldS._mean - newS._mean)
              + stat->_count * (stat->_mean - newS._mean)
                  * (stat->_mean - newS._mean))
              / (newS._count)
      );
      newS._var = newS._stdv * newS._stdv;
      oldS = newS;
    }
    return oldS._stdv;
  }

  long long getGlobalCount() {
    return (long long) std::accumulate(
        _values.begin(), _values.end(), 0,
        [](long long sum, const Statistic &stat) { return stat._count + sum; }
    );
  }

  friend std::ostream &operator<<(std::ostream &os,
                                  const Statistics &statistics) {
    for (auto &_value : statistics._values) {
      os << _value;
    }
    return os;
  }

};

#endif //HTGS_FAST_IMAGE_STATISTICS_H
