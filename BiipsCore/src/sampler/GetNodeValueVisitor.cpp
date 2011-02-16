//                                               -*- C++ -*-
/*! \file GetNodeValueVisitor.cpp
 * \brief
 *
 * \author  $LastChangedBy$
 * \date    $LastChangedDate$
 * \version $LastChangedRevision$
 * Id:      $Id$
 */

#include "graph/NodeVisitor.hpp"
#include "sampler/GetNodeValueVisitor.hpp"
#include "sampler/NodeSampler.hpp"
#include "graph/ConstantNode.hpp"
#include "graph/LogicalNode.hpp"
#include "graph/StochasticNode.hpp"
#include "graph/Graph.hpp"

namespace Biips
{

  class GetNodeValueVisitor : public ConstNodeVisitor
  {
  protected:
    NodeSampler * pSampleNodeVis_;
    DataType value_;

  public:
    virtual void Visit(const ConstantNode & node);
    virtual void Visit(const StochasticNode & node);
    virtual void Visit(const LogicalNode & node);

    const DataType & GetValue() const { return value_; };

    explicit GetNodeValueVisitor(NodeSampler * pSampleNodeVis) : pSampleNodeVis_(pSampleNodeVis) {};
  };


  void GetNodeValueVisitor::Visit(const ConstantNode & node)
  {
    if ( nodeIdDefined_ ) // TODO manage else case : throw exception
    {
      value_ = DataType(node.DimPtr(), pSampleNodeVis_->nodeValuesMap_[nodeId_]);
    }
  }


  void GetNodeValueVisitor::Visit(const StochasticNode & node)
  {
      if ( nodeIdDefined_ ) // TODO manage else case : throw exception
      {
        if ( pSampleNodeVis_->sampledFlagsMap_[nodeId_] )// TODO manage else case : throw exception
        {
          value_ = DataType(node.DimPtr(), pSampleNodeVis_->nodeValuesMap_[nodeId_]);
        }
      }
  }


  void GetNodeValueVisitor::Visit(const LogicalNode & node)
  {
    if ( nodeIdDefined_ ) // TODO manage else case : throw exception
    {
      if ( !pSampleNodeVis_->sampledFlagsMap_[nodeId_] )
      {
        NodeId snv_node_id = pSampleNodeVis_->nodeId_;
        pSampleNodeVis_->SetNodeId(nodeId_);
        node.AcceptVisitor(*pSampleNodeVis_);
        pSampleNodeVis_->SetNodeId(snv_node_id);
      }
      value_ = DataType(node.DimPtr(), pSampleNodeVis_->nodeValuesMap_[nodeId_]);
    }
  }


  DataType getNodeValue(NodeId nodeId, const Graph * pGraph, NodeSampler * pNodeSampler)
  {
    GetNodeValueVisitor get_val_vis(pNodeSampler);
    pGraph->VisitNode(nodeId, get_val_vis);
    return get_val_vis.GetValue();
  }


  DataType::Array getParamValues(NodeId nodeId, const Graph * pGraph, NodeSampler * pSampleNodeVis)
  {
    GraphTypes::DirectParentNodeIdIterator it_param, it_param_end;
    boost::tie(it_param, it_param_end) = pGraph->GetParents(nodeId);
    DataType::Array param_values(std::distance(it_param, it_param_end));
    Size i = 0;
    while (it_param != it_param_end)
    {
      param_values[i] = getNodeValue(*it_param, pGraph, pSampleNodeVis);
      ++it_param;
      ++i;
    }
    return param_values;
  }

}
