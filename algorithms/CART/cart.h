#ifndef CART_H
#define CART_H

#include <vector>
#include <memory>
#include <map>
#include "../decisiontree.h"

class CARTNode;
class IAlgorithmDataSource;
class CARTSplitCriterion;

/** The CART class represents the CART algorithm, which serves to build decision trees from data to classify or
 * to perform regressions.  CART stands for Classification and Regression Tree.
 */
class CART : public DecisionTree
{
public:

    /** Builds a CART tree using the given data set and passing a list of column IDs
     * corresponding to the features/variables to use as training data.  The root of
     * the resulting CART tree is pointed by the m_root member.
     * @param trainingData Reference to the training data set object.
     * @param outputData Reference to the data set to be classified or estimated.  Since it is read-only, it is up to the
     *                   calling code to make updates to the output data source after calling classify() or regress().
     * @param trainingFeatureIDs List of column numbers corresponding to the selected predictive
     *                           variables (features) in the training set.
     * @param outputFeatureIDs List of column numbers corresponding to the selected predictive
     *                           variables (features) in the output set.
     * @param continuousFeatureMaxSplits Limits the number of split values for continuous variables.
     */
    CART( const IAlgorithmDataSource& trainingData,
          const IAlgorithmDataSource& outputData,
          const std::vector<int> &trainingFeatureIDs,
          const std::vector<int> &outputFeatureIDs,
          int continuousFeaturesMaxSplits );

    virtual ~CART();

    /** Uses the underlying CART decision tree as a classifier to a given output data row, referenced by its row number.
     * This is just a front-end for the actual recursive classification function (see the protected section).
     * @param rowIdOutput Row number of output data to classify.
     * @param dependentVariableColumnID  The column id in the training data of the variable (supposedly categorical)
     *                                   to be predicted.
     * @param result A list with pair(s): the predicted value; how many times the value was found in training data.
     */
    virtual void classify(long rowIdOutput,
                           int dependentVariableColumnID,
                           std::vector<std::pair<DataValue, long> > &result) const override;

    /** Uses the underlying CART decision tree as a regression to a given output data row, referenced by its row number.
     * This is just a front-end for the actual recursive regression function (see the protected section).
     * @param rowIdOutput Row number of output data to produce an estimation.
     * @param dependentVariableColumnID  The column id in the training data of the variable (supposedly continuous)
     *                                   to be predicted.
     * @param mean The estimated result.
     * @param percent The percentage of the training data rows that was used in the regression decision.  This value is
     *                a measure of representativeness of the returned mean.
     */
    virtual void regress( long rowIdOutput,
                          int dependentVariableColumnID,
                          DataValue &mean,
                          double &percent ) const override;

protected:

    /** The root of the CART tree. */
    std::shared_ptr<CARTNode> m_root;

    /** The data from which the CART tree is built. */
    const IAlgorithmDataSource& m_trainingData;

    /** The data to receive predictions. */
    const IAlgorithmDataSource& m_outputData;

    /** This map object maps feature column indexes in the training data set to feature column indexes in the output data set.
     * This is necessary because rarely a given feature has exactly the same column index in both data sets.
     * This list is built in the constructor.
     */
    std::map<int,int> m_training2outputFeatureIndexesMap;

    /** Limit to the number of split values for continuous features. */
    int m_continuousFeaturesMaxSplits;

    /* The functions below are arranged in dependency order. Of course the recursive functions depend
       on themselves. */

    /** Returns a collection of the unique values found in the given column of a set of
     * rows (given by a list of row numbers).  The passed list is reset.
     * VERIFIED.
     */
    void getUniqueDataValues(std::vector<DataValue> &result,
                              const std::vector<long> &rowIDs,
                              int columnIndex ) const;

    /** Counts the unique values found in the given column.  Normally used for categorical values.
     * This method resets the passed list.
     * @param result A list of pairs: the first value is a categorical value that was counted.  The second
     *               value will hold the count.
     * @param rowIDs A list of row numbers from wich to take the counts.
     * @param columnIndex The index of a column with the values.  See isContinuous().
     * VERIFIED.
     */
    void getUniqueValuesCounts(std::vector<std::pair<DataValue, long> > &result,
                              const std::vector<long> &rowIDs,
                              int columnIndex ) const;

    /**
     * Computes the Gini impurity factor for a given variable in a set of rows.
     * This factor is a likelyhood of being incorrect in picking a category.  A factor of zero means you will
     * always pick the correct class (all class values in the column are the same).
     * VERIFIED.
     */
    double getGiniImpurity(const std::vector<long> &rowIDs,
                           int columnIndex ) const ;

    /**
     * Calculates the information gain given a data split and the ammout of uncertainty before the split.
     * If the returned value is positive, than the proposed split will decrease uncertainty.
     * @param rowIDsTrueSide Row IDs of data that matched the split criterion.
     * @param rowIDsFalseSide Rod IDs of data that did not match the split criterion.
     * @param columnIndex The column ID of the variable used in the split criterion.
     * @param impurityFactorBeforeTheSplit The uncertainty measure between 0.0 and 1.0 before the split.
     * VERIFIED.
     */
    double getSplitInformationGain(const std::vector<long> &rowIDsTrueSide,
                                    const std::vector<long> &rowIDsFalseSide,
                                    int columnIndex,
                                    double impurityFactorBeforeTheSplit ) const;

    /**
     * Performs data split for the CART algorithm.  The output lists are cleared before splitting.
     * @param rowIDs The set of row id's to be split.
     * @param criterion The split criterion.
     * @param trueSideRowIDs The set of ids of rows that match the criterion.
     * @param falseSideRowIDs The set of ids of rows that don't match the criterion.
     * VERIFIED.
     */
    void split(const std::vector<long> &rowIDs,
               const CARTSplitCriterion &criterion,
               std::vector<long> &trueSideRowIDs,
               std::vector<long> &falseSideRowIDs ) const;

    /**
     * Reduces the given vector of data values by skipping elements at fixed steps.
     * If the vector is smaller than the target maximum size, no change takes place.
     */
    void decimate(std::vector<DataValue>& values, unsigned long maxSize ) const;

    /**
     * Returns the CART tree partition criterion with the highest information gain among the possible ones
     * that can be made with the data given by row numbers (IDs).  Information gain is defined by reduction
     * of uncertainty (sum or impurity) in the tree nodes below.  The goal is to get large data subsets with
     * low uncertainty until we get leaf nodes with pure (0% chance of incorrect picking) or at least with
     * low impurity.
     * @param rowIDs Row IDs of the row set.
     * @param featureIDs Column IDs of the variables/features participating in the training data.
     */
    std::pair<CARTSplitCriterion, double> getSplitCriterionWithMaximumInformationGain(const std::vector<long> &rowIDs,
                                                                                      const std::vector<int> &featureIDs) const;

    /** Builds a CART tree hierarchy from the given set of data rows (referenced by a list of row numbers).
     * @param rowIDs Row IDs of the row set.
     * @param featureIDs Column IDs of the variables/features participating in the training data.
     */
    CARTNode* makeCART(const std::vector<long> &rowIDs , const std::vector<int> &featureIDs ) const;

    /** The actual recursive implementation of classify().
     * @param decisionTreeNode the node of the tree holding the decision hierarchy to classify.
     *                         If nullptr, the root node is used.
     */
    void classify(long rowIdOutput,
                   int dependentVariableColumnID,
                   const CARTNode* decisionTreeNode,
                   std::vector<std::pair<DataValue, long> > &result) const;

    /** The actual recursive implementation of regress().
     * @param decisionTreeNode the node of the tree holding the decision hierarchy to classify.
     *                         If nullptr, the root node is used.
     */
    void regress(long rowIdOutput,
                  int dependentVariableColumnID,
                  const CARTNode* decisionTreeNode,
                  DataValue &mean,
                  double &percent ) const;
};

#endif // CART_H
