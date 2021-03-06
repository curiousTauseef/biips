#ifndef BIIPS_CONJUGATESAMPLER_HPP_
#define BIIPS_CONJUGATESAMPLER_HPP_

#include "sampler/NodeSampler.hpp"
#include "graph/StochasticNode.hpp"
#include "graph/Graph.hpp"
#include "sampler/NodesRelationVisitor.hpp"
#include "sampler/GetNodeValueVisitor.hpp"
#include "sampler/LogLikeVisitor.hpp"

namespace Biips
{

  template<class ConjugateSampler>
  class ConjugateSamplerFactory;

  template<typename ConjugateSampler>
  class LikeFormVisitor;

  template<typename PriorDist, typename LikeDist, Size paramIndex>
  class ConjugateSampler: public NodeSampler
  {
  public:
    typedef ConjugateSampler<PriorDist, LikeDist, paramIndex> SelfType;
    typedef typename Types<SelfType>::Ptr Ptr;
    typedef PriorDist PriorDistType;
    typedef LikeDist LikeDistType;

  protected:
    friend class ConjugateSamplerFactory<SelfType> ;
    friend class LikeFormVisitor<SelfType> ;

    explicit ConjugateSampler(const Graph & graph) :
        NodeSampler(graph)
    {
    }

    virtual MultiArray::Array initLikeParamContrib() const;
    virtual void
    formLikeParamContrib(NodeId likeId,
                         MultiArray::Array & likeParamContribValues) = 0;
    virtual MultiArray::Array
    postParam(const NumArray::Array & priorParamContribValues,
              const MultiArray::Array & likeParamContribValues) const = 0;
    virtual Scalar
    computeLogIncrementalWeight(const NumArray & sampledData,
                                const NumArray::Array & priorParamValues,
                                const NumArray::Array & postParamValues,
                                const MultiArray::Array & likeParamContrib =
                                    MultiArray::Array());
    virtual void sample(const StochasticNode & node);

  public:
    static Bool IsConjugate(const StochasticNode & node,
                            NodeId priorId,
                            const Graph & graph);

    virtual const String & Name() const = 0;

    virtual ~ConjugateSampler()
    {
    }
  };

  template<typename ConjugateSampler>
  class ConjugateSamplerFactory: public NodeSamplerFactory
  {
  public:
    typedef ConjugateSamplerFactory<ConjugateSampler> SelfType;
    typedef typename Types<SelfType>::Ptr Ptr;
    typedef ConjugateSampler CreatedType;
    typedef NodeSamplerFactory BaseType;
    typedef typename ConjugateSampler::PriorDistType PriorDistType;
    typedef typename ConjugateSampler::LikeDistType LikeDistType;

  protected:
    static Ptr pConjugateSamplerFactoryInstance_;
    ConjugateSamplerFactory()
    {
    }

  public:
    static BaseType::Ptr Instance()
    {
      return pConjugateSamplerFactoryInstance_;
    }
    virtual Bool Create(const Graph & graph,
                        NodeId nodeId,
                        BaseType::CreatedPtr & pNodeSamplerInstance) const;
    virtual ~ConjugateSamplerFactory()
    {
    }
  };

  template<typename PriorDist, typename LikeDist, Size paramIndex>
  MultiArray::Array ConjugateSampler<PriorDist, LikeDist, paramIndex>::initLikeParamContrib() const
  {
    Size n_par = PriorDist::Instance()->NParam();
    MultiArray::Array paramContribValues(n_par);

    GraphTypes::ParentIterator it_parents, it_parents_end;
    boost::tie(it_parents, it_parents_end) = graph_.GetParents(nodeId_);
    for (Size i = 0; i < n_par; ++it_parents, ++i)
    {
      DimArray::Ptr dim_ptr = graph_.GetNode(*it_parents).DimPtr();
      ValArray::Ptr val_ptr(new ValArray(dim_ptr->Length()));
      paramContribValues[i].SetPtr(dim_ptr, val_ptr);
    }
    return paramContribValues;
  }

  template<typename PriorDist, typename LikeDist, Size paramIndex>
  Scalar ConjugateSampler<PriorDist, LikeDist, paramIndex>::computeLogIncrementalWeight(const NumArray & sampledData,
                                                                                        const NumArray::Array & priorParamValues,
                                                                                        const NumArray::Array & postParamValues,
                                                                                        const MultiArray::Array & likeParamContrib)
  {
    // Prior
    Scalar log_prior = PriorDist::Instance()->LogDensity(sampledData,
                                                         priorParamValues,
                                                         NULL_NUMARRAYPAIR); // FIXME Boundaries
    if (isNan(log_prior))
      throw NodeError(nodeId_, "Failure to calculate log prior density.");

    // Likelihood
    Scalar log_like = getLogLikelihood(graph_, nodeId_, *this);

    // Posterior
    Scalar log_post = PriorDist::Instance()->LogDensity(sampledData,
                                                        postParamValues,
                                                        NULL_NUMARRAYPAIR); // FIXME Boundaries
    if (isNan(log_post))
      throw NodeError(nodeId_, "Failure to calculate log posterior density.");

    // Incremental weight
    Scalar log_incr_weight = log_prior + log_like - log_post;
    if (isNan(log_incr_weight))
    {
      if (!isFinite(log_prior))
      {
        if (!isFinite(log_like))
          throw RuntimeError("Prior and likelihood are incompatible.");
        if (!isFinite(log_post))
          throw RuntimeError("Prior and posterior are incompatible.");
      }

      if (!isFinite(log_like))
      {
        if (!isFinite(log_post))
          throw RuntimeError("Likelihood and posterior are incompatible.");
      }

      throw RuntimeError("Failure to calculate log incremental weight.");
    }

    // TODO optimize computation removing constant terms
    return log_incr_weight;
  }

  template<typename ConjugateSampler>
  class LikeFormVisitor: public ConstNodeVisitor
  {
  protected:
    typedef LikeFormVisitor<ConjugateSampler> SelfType;
    typedef typename Types<SelfType>::Ptr Ptr;
    typedef typename ConjugateSampler::PriorDistType PriorDistType;

    const Graph & graph_;
    ConjugateSampler & nodeSampler_;
    MultiArray::Array & paramContribValues_;

    virtual void visit(const StochasticNode & node)
    {
      nodeSampler_.formLikeParamContrib(nodeId_, paramContribValues_);
    }

  public:
    LikeFormVisitor(const Graph & graph,
                    ConjugateSampler & nodeSampler,
                    MultiArray::Array & paramContribValues) :
        graph_(graph), nodeSampler_(nodeSampler), paramContribValues_(paramContribValues)
    {
    }
  };

  template<typename PriorDist, typename LikeDist, Size paramIndex>
  void ConjugateSampler<PriorDist, LikeDist, paramIndex>::sample(const StochasticNode & node)
  {
    NumArray::Array prior_param_values = getParamValues(nodeId_, graph_, *this);

    GraphTypes::LikelihoodChildIterator it_offspring, it_offspring_end;
    boost::tie(it_offspring, it_offspring_end) =
        graph_.GetLikelihoodChildren(nodeId_);

    MultiArray::Array like_param_contrib = initLikeParamContrib();
    LikeFormVisitor<SelfType> like_form_vis(graph_, *this, like_param_contrib);
    for (; it_offspring != it_offspring_end; ++it_offspring)
    {
      graph_.VisitNode(*it_offspring, like_form_vis);
    }

    MultiArray::Array post_param_values = postParam(prior_param_values,
                                                    like_param_contrib);

    // allocate memory
    nodeValuesMap()[nodeId_].reset(new ValArray(node.Dim().Length()));

    // sample
    PriorDist::Instance()->Sample(*nodeValuesMap()[nodeId_],
                                  NumArray::Array(post_param_values),
                                  NULL_NUMARRAYPAIR,
                                  *pRng_); // FIXME Boundaries
    sampledFlagsMap()[nodeId_] = true;

    NumArray sampled_data(node.DimPtr().get(), nodeValuesMap()[nodeId_].get());

    logIncrementalWeight_ = computeLogIncrementalWeight(sampled_data,
                                                        prior_param_values,
                                                        NumArray::Array(post_param_values),
                                                        like_param_contrib);
  }

  template<typename PriorDist, typename LikeDist, Size paramIndex>
  Bool ConjugateSampler<PriorDist, LikeDist, paramIndex>::IsConjugate(const StochasticNode & node,
                                                                      NodeId priorId,
                                                                      const Graph & graph)
  {
    Bool conjugate = false;
    const Types<NodeId>::Array & params = node.Parents();
    if ((node.PriorName() == LikeDist::Instance()->Name())
        && (params[paramIndex] == priorId))
    {
      conjugate = true;
      for (Size i = 0; i < params.size(); ++i)
      {
        if (i != paramIndex)
          conjugate = (nodesRelation(params[i], priorId, graph) == KNOWN);
        if (!conjugate)
          break;
      }
    }
    return conjugate;
  }

  template<typename ConjugateSampler>
  class IsConjugateVisitor: public ConstNodeVisitor
  {
  protected:
    const Graph & graph_;
    const NodeId priorId_;
    Bool conjugate_;

    void visit(const StochasticNode & node)
    {
      conjugate_ = false;

      // FIXME
      if (node.IsBounded())
        return;

      conjugate_ = ConjugateSampler::IsConjugate(node, priorId_, graph_);
    }

  public:
    Bool IsConjugate() const
    {
      return conjugate_;
    }

    IsConjugateVisitor(const Graph & graph, NodeId myId) :
        graph_(graph), priorId_(myId), conjugate_(false)
    {
    }
  };

  template<typename ConjugateSampler>
  class CanSampleVisitor: public ConstNodeVisitor
  {

  protected:
    typedef typename ConjugateSampler::PriorDistType PriorDistType;

    const Graph & graph_;
    Bool canSample_;

    void visit(const StochasticNode & node)
    {
      canSample_ = false;

      if (graph_.GetObserved()[nodeId_])
        throw LogicError("CanSampleVisitor can not visit observed node: node id sequence of the forward sampler may be bad.");

      if (node.PriorName() != PriorDistType::Instance()->Name())
        return;

      // FIXME
      if (node.IsBounded())
        return;

      GraphTypes::LikelihoodChildIterator it_offspring, it_offspring_end;
      boost::tie(it_offspring, it_offspring_end) =
          graph_.GetLikelihoodChildren(nodeId_);

      IsConjugateVisitor<ConjugateSampler> child_vis(graph_, nodeId_);

      for (; it_offspring != it_offspring_end; ++it_offspring)
      {
        graph_.VisitNode(*it_offspring, child_vis);
        canSample_ = child_vis.IsConjugate();
        if (!canSample_)
          break;
      }
    }

  public:
    Bool CanSample() const
    {
      return canSample_;
    }

    CanSampleVisitor(const Graph & graph) :
        graph_(graph), canSample_(false)
    {
    }
  };

  template<typename ConjugateSampler>
  Bool ConjugateSamplerFactory<ConjugateSampler>::Create(const Graph & graph,
                                                         NodeId nodeId,
                                                         BaseType::CreatedPtr & pNodeSamplerInstance) const
  {
    CanSampleVisitor<CreatedType> can_sample_vis(graph);

    graph.VisitNode(nodeId, can_sample_vis);

    Bool flag_created = can_sample_vis.CanSample();

    if (flag_created)
      pNodeSamplerInstance = BaseType::CreatedPtr(new CreatedType(graph));

    return flag_created;
  }

  template<typename ConjugateSampler>
  typename ConjugateSamplerFactory<ConjugateSampler>::Ptr ConjugateSamplerFactory<
      ConjugateSampler>::pConjugateSamplerFactoryInstance_(new SelfType());

}

#endif /* BIIPS_CONJUGATESAMPLER_HPP_ */
