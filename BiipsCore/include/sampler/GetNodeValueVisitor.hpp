//                                               -*- C++ -*-
/*! \file GetNodeValueVisitor.hpp
 * \brief
 *
 * \author  $LastChangedBy$
 * \date    $LastChangedDate$
 * \version $LastChangedRevision$
 * Id:      $Id$
 */

#ifndef BIIPS_GETNODEVALUEVISITOR_HPP_
#define BIIPS_GETNODEVALUEVISITOR_HPP_

namespace Biips
{
  class Graph;
  class NodeSampler;

  DataType getNodeValue(NodeId nodeId, const Graph * pGraph, NodeSampler * pNodeSampler);

  DataType::Array getParamValues(NodeId nodeId, const Graph * pGraph, NodeSampler * pSampleNodeVis);
}


#endif /* BIIPS_GETNODEVALUEVISITOR_HPP_ */
