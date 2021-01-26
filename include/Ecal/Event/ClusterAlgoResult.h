/**
 * @file ClusterAlgoResult.h
 * @brief Class that holds details about the clustering algorithm as a whole
 * @author Josh Hiltbrand, University of Minnesota
 *
 */

#ifndef EVENT_CLUSTERALGORESULT_H_
#define EVENT_CLUSTERALGORESULT_H_

// ROOT
#include "TArrayD.h"
#include "TObject.h"  //For ClassDef
#include "TString.h"

// STL
#include <iostream>

namespace ldmx { 

/**
 * @class ClusterAlgoResult
 * @brief Contains details about the clustering algorithm
 */
class ClusterAlgoResult {
 public:
  /**
   * Class constructor.
   */
  ClusterAlgoResult();

  /**
   * Class destructor.
   */
  virtual ~ClusterAlgoResult();

  /**
   * Print a description of this object.
   */
  void Print() const;

  /**
   * Reset the ClusterAlgoResult object.
   */
  void Clear();

  /**
   * Return the name of the cluster algo.
   * @return The name of the cluster algo.
   */
  const TString& getName() const { return name_; }

  /**
   * Return algorithm variable i (see algorithm code for details).
   * @param element The index of the variable.
   * @return Algorithm variable at the index.
   */
  double getAlgoVar(int element) const { return variables_[element]; }

  /**
   * Return algorithm variable 0 (see algorithm code for details).
   * @note Provided for interactive ROOT use.
   * @return Algorithm variable 0.
   */
  double getAlgoVar0() const {
    return (variables_.GetSize() < 1) ? (0) : (variables_[0]);
  }

  /**
   * Return algorithm variable 1 (see algorithm code for details).
   * @note Provided for interactive ROOT use.
   * @return Algorithm variable 1.
   */
  double getAlgoVar1() const {
    return (variables_.GetSize() < 2) ? (0) : (variables_[1]);
  }

  /**
   * Return algorithm variable 2 (see algorithm code for details).
   * @note Provided for interactive ROOT use.
   * @return Algorithm variable 2.
   */
  double getAlgoVar2() const {
    return (variables_.GetSize() < 3) ? (0) : (variables_[2]);
  }

  /**
   * Return algorithm variable 3 (see algorithm code for details).
   * @note Provided for interactive ROOT use.
   * @return Algorithm variable 3.
   */
  double getAlgoVar3() const {
    return (variables_.GetSize() < 4) ? (0) : (variables_[3]);
  }

  /**
   * Return algorithm variable 4 (see algorithm code for details).
   * @note Provided for interactive ROOT use.
   * @return Algorithm variable 4.
   */
  double getAlgoVar4() const {
    return (variables_.GetSize() < 5) ? (0) : (variables_[4]);
  }

  /**
   * Return the weight reached when cluster count reached n.
   * @return The weight from the cluster algorithm.
   */
  double getWeight(int nCluster) const {
    return (nCluster < weights_.GetSize()) ? (0) : (weights_[nCluster]);
  }

  /**
   * Set name and number of variables of cluster algo.
   * @param name The name of the cluster algo.
   * @param nvar The number of algorithm variables.
   */
  void set(const TString& name, int nvar);

  /**
   * Set name, number of variables and weights of cluster algo.
   * @param name The name of the cluster algo.
   * @param nvar The number of algorithm variables.
   * @param nweights The number of transition weights.
   */
  void set(const TString& name, int nvar, int nweights);

  /**
   * Set an algorithm variable.
   * @param element The index of the variable.
   * @param value The variable's new value.
   */
  void setAlgoVar(int element, double value);

  /**
   * Set a weight when number of clusters reached.
   * @param nClusters The number of clusters reached.
   * @param weight The weight when number of clusters reached.
   */
  void setWeight(int nClusters, double weight);

 private:
  /** Name of the clustering algorithm. */
  TString name_{};

  /* Algorithm variable results from the cluster algo. */
  TArrayD variables_;

  /* Array of weights when a certain number of clusters is reached. */
  TArrayD weights_;

  ClassDef(ClusterAlgoResult, 1);
};
}  // namespace ldmx

#endif
