#ifndef LIBLEARNING_RANDOMFOREST_FOREST_H
#define LIBLEARNING_RANDOMFOREST_FOREST_H 
#include "common-libraries.hpp"
#include "tree.hpp"
#include <boost/ptr_container/serialize_ptr_vector.hpp>
#if VERBOSE_TREE_PROGRESS
#include <cstdio>
#endif

#include "IO_Binary.hpp"

#include <boost/random/random_number_generator.hpp>
#include <boost/random.hpp>
#include <boost/random/linear_congruential.hpp>

namespace cpp98 {

// Reimplementation of std::random_shuffle, for the use of Spatial_sorting.
// We want an implementation of random_shuffle that produces the same
// result on all platforms, for a given seeded random generator.
template <class RandomAccessIterator,
          class RandomGenerator>
void
random_shuffle(RandomAccessIterator begin, RandomAccessIterator end,
               RandomGenerator& random)
{
  if(begin == end) return;
  for(RandomAccessIterator it = begin + 1; it != end; ++it)
  {
    std::iter_swap( it, begin + random( (it - begin) + 1 ) );
    // The +1 inside random is because random(N) gives numbers in the open
    // interval [0, N[
  }
}

template <class RandomAccessIterator>
void
random_shuffle(RandomAccessIterator begin, RandomAccessIterator end)
{
  typedef std::iterator_traits<RandomAccessIterator> Iterator_traits;
  typedef typename Iterator_traits::difference_type Diff_t;
  boost::rand48 random;
  boost::random_number_generator<boost::rand48, Diff_t> rng(random);
  cpp98::random_shuffle(begin,end, rng);
}

} // namespace cpp98

// DHL

namespace liblearning {
namespace RandomForest {

// DHL
template <typename NodeT, typename SplitGenerator>
class Tree_training_functor
{
  typedef typename NodeT::ParamType ParamType;
  typedef typename NodeT::FeatureType FeatureType;
  typedef Tree<NodeT> TreeType;

  std::size_t seed_start;
  const std::vector<int>& sample_idxes;
  boost::ptr_vector<Tree<NodeT> >& trees;
  DataView2D<FeatureType> samples;
  DataView2D<int> labels;
  std::size_t n_in_bag_samples;
  const SplitGenerator& split_generator;

public:

  Tree_training_functor(std::size_t seed_start,
                        const std::vector<int>& sample_idxes,
                        boost::ptr_vector<Tree<NodeT> >& trees,
                        DataView2D<FeatureType> samples,
                        DataView2D<int> labels,
                        std::size_t n_in_bag_samples,
                        const SplitGenerator& split_generator)
    : seed_start (seed_start)
    , sample_idxes (sample_idxes)
    , trees (trees)
    , samples (samples)
    , labels (labels)
    , n_in_bag_samples(n_in_bag_samples)
    , split_generator(split_generator)
  { }

//#ifdef CGAL_LINKED_WITH_TBB
//  void operator()(const tbb::blocked_range<std::size_t>& r) const
//  {
//    for (std::size_t s = r.begin(); s != r.end(); ++ s)
//      apply(s);
//  }
//#endif // CGAL_LINKED_WITH_TBB

  inline void apply (std::size_t i_tree) const
  {
    // initialize random generator with sequential seeds (one for each
    // tree)
    RandomGen gen(seed_start + i_tree);
    std::vector<int> in_bag_samples = sample_idxes;

    // Bagging: draw random sample indexes used for this tree
    cpp98::random_shuffle (in_bag_samples.begin(),in_bag_samples.end());

    // Train the tree
    trees[i_tree].train(samples, labels, &in_bag_samples[0], n_in_bag_samples, split_generator, gen);
  }

};
// DHL

template <typename NodeT>
class RandomForest {
public:
    typedef typename NodeT::ParamType ParamType;
    typedef typename NodeT::FeatureType FeatureType;
    typedef Tree<NodeT> TreeType;
    ParamType params;

    std::vector<uint8_t> was_oob_data;
    DataView2D<uint8_t> was_oob;

    boost::ptr_vector< Tree<NodeT> > trees;

    RandomForest() {}
    RandomForest(ParamType const& params) : params(params) {}

    template<typename SplitGenerator>
    void train(DataView2D<FeatureType> samples, 
               DataView2D<int> labels, 
               DataView2D<int> train_sample_idxes, 
               SplitGenerator const& split_generator,
               size_t seed_start = 1,
               // DHL
               std::size_t n_classes = std::size_t(-1)
//               bool register_oob = true
               // DHL
               ) 
    {
        trees.clear();
        // DHL
//        params.n_classes  = *std::max_element(&labels(0,0), &labels(0,0)+labels.num_elements()) + 1;
        if (n_classes == std::size_t(-1))
          params.n_classes = *std::max_element(&labels(0,0), &labels(0,0)+labels.num_elements()) + 1;
        else
          params.n_classes = n_classes;
        // DHL
        params.n_features = samples.cols;
        params.n_samples  = samples.rows;

        std::vector<int> sample_idxes;

        if (train_sample_idxes.empty()) {
            // no indexes were passed, generate vector with all indexes
            sample_idxes.resize(params.n_samples);
            for (size_t i_sample = 0; i_sample < params.n_samples; ++i_sample) {
                sample_idxes[i_sample] = i_sample;
            }
        } else {
            // copy indexes
            sample_idxes.assign(&train_sample_idxes(0,0), &train_sample_idxes(0,0)+train_sample_idxes.num_elements());
        }

        size_t n_idxes = sample_idxes.size();
        params.n_in_bag_samples = n_idxes * (1 - params.sample_reduction);

        // DHL
        std::size_t nb_trees = trees.size();
        for (std::size_t i_tree = nb_trees; i_tree < nb_trees + params.n_trees; ++ i_tree)
          trees.push_back (new TreeType(&params));

        Tree_training_functor<NodeT, SplitGenerator>
          f (seed_start, sample_idxes, trees, samples, labels, params.n_in_bag_samples, split_generator);

//#ifndef CGAL_LINKED_WITH_TBB
//        CGAL_static_assertion_msg (!(std::is_convertible<ConcurrencyTag, Parallel_tag>::value),
//                                   "Parallel_tag is enabled but TBB is unavailable.");
//#else
//        if (std::is_convertible<ConcurrencyTag,Parallel_tag>::value)
//        {
//          tbb::parallel_for(tbb::blocked_range<size_t>(nb_trees, nb_trees + params.n_trees), f);
//        }
//        else
//#endif
        {
          for (size_t i_tree = nb_trees; i_tree < nb_trees + params.n_trees; ++i_tree)
          {
//#if VERBOSE_TREE_PROGRESS
//            std::printf("Training tree %zu/%zu, max depth %zu\n", i_tree+1, nb_trees + params.n_trees, params.max_depth);
//#endif
            f.apply(i_tree);
          }
        }

        /*
        // Random distribution over indexes
        UniformIntDist dist(0, n_idxes - 1);

        // Store for each sample and each tree if sample was used for tree
        if (register_oob) {
            was_oob_data.assign(n_idxes*params.n_trees, 1);
            was_oob = DataView2D<uint8_t>(&was_oob_data[0], n_idxes, params.n_trees);
        }

        for (size_t i_tree = 0; i_tree < params.n_trees; ++i_tree) {
#if VERBOSE_TREE_PROGRESS
            std::printf("Training tree %zu/%zu, max depth %zu\n", i_tree+1, params.n_trees, params.max_depth);
#endif
            // new tree
            trees.push_back(new TreeType(&params));
            // initialize random generator with sequential seeds (one for each
            // tree)
            RandomGen gen(seed_start + i_tree);
            // Bagging: draw random sample indexes used for this tree
            std::vector<int> in_bag_samples(params.n_in_bag_samples);
            for (size_t i_sample = 0; i_sample < in_bag_samples.size(); ++i_sample) {
                int random_idx = dist(gen);
                in_bag_samples[i_sample] = sample_idxes[random_idx];
                if (register_oob && was_oob(random_idx, i_tree)) {
                    was_oob(random_idx, i_tree) = 0;
                }
            }
#ifdef TREE_GRAPHVIZ_STREAM
            TREE_GRAPHVIZ_STREAM << "digraph Tree {" << std::endl;
#endif
            // Train the tree
            trees.back().train(samples, labels, &in_bag_samples[0], in_bag_samples.size(), split_generator, gen);
#ifdef TREE_GRAPHVIZ_STREAM
            TREE_GRAPHVIZ_STREAM << "}" << std::endl << std::endl;
#endif
        }
        */
        // DHL
    }
    int evaluate(FeatureType const* sample, float* results) {
        // initialize output probabilities to 0
        // DHL
        std::fill_n(results, params.n_classes, 0.0f);
//        std::fill_n(results, params.n_classes, 0);
        // DHL
        // accumulate votes of the trees
        for (size_t i_tree = 0; i_tree < trees.size(); ++i_tree) {
            float const* tree_result = trees[i_tree].evaluate(sample);
            for (size_t i_cls = 0; i_cls < params.n_classes; ++i_cls) {
                results[i_cls] += tree_result[i_cls];
            }
        }
        float best_val   = 0.0;
        int   best_class = 0;
        float scale      = 1.0 / trees.size();
        for (size_t i_cls = 0; i_cls < params.n_classes; ++i_cls) {
            // divide by number of trees to normalize probability
            results[i_cls] *= scale;
            // determine best class
            if (results[i_cls] > best_val) {
                best_val = results[i_cls];
                best_class = i_cls;
            }
        }
        return best_class;
    }
#if 0
    float similarity_endnode(float const* sample_1, float const* sample_2) {
        double sum = 0.0;
        for (size_t i_tree = 0; i_tree < trees.size(); ++i_tree) {
            sum += trees[i_tree].similarity_endnode(sample_1, sample_2);
        }
        return sum/trees.size();
    }
    float similarity_path(float const* sample_1, float const* sample_2) {
        double sum = 0.0;
        for (size_t i_tree = 0; i_tree < trees.size(); ++i_tree) {
            sum += trees[i_tree].similarity_path(sample_1, sample_2);
        }
        return sum/trees.size();
    }
#endif
    template <typename Archive>
    void serialize(Archive& ar, unsigned /* version */)
    {
        ar & BOOST_SERIALIZATION_NVP(params);
        ar & BOOST_SERIALIZATION_NVP(trees);
    }
// DHL
    void write (std::ostream& os)
    {
      params.write(os);

      IO_Binary::I_Binary_write_size_t_into_uinteger32 (os, trees.size());
      for (std::size_t i_tree = 0; i_tree < trees.size(); ++i_tree)
        trees[i_tree].write(os);
    }

    void read (std::istream& is)
    {
      params.read(is);

      std::size_t nb_trees;
      IO_Binary::I_Binary_read_size_t_from_uinteger32 (is, nb_trees);
      for (std::size_t i = 0; i < nb_trees; ++ i)
      {
        trees.push_back (new TreeType(&params));
        trees.back().read(is);
      }
    }

    void get_feature_usage (std::vector<std::size_t>& count) const
    {
      for (std::size_t i_tree = 0; i_tree < trees.size(); ++i_tree)
        trees[i_tree].get_feature_usage(count);
    }
// DHL
};

}
}
#endif
