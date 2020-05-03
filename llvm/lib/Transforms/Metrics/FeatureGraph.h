#ifndef LLVM_LIB_TRANSFORM_METRICS_FEATUREGRAPH_H
#define LLVM_LIB_TRANSFORM_METRICS_FEATUREGRAPH_H

#define DEBUG_TYPE "metrics"

namespace llvm {
class MNode;
class MEdge;
using MNodeBase = DGNode<MNode, MEdge>;
using MEdgeBase = DGEdge<MNode, MEdge>;
using MDGBase = DirectedGraph<MNode, MEdge>;

struct NodeFeatures {
  unsigned MemOps = 0;
  unsigned TotalMemAccessInBytes = 0;
  unsigned ExecutionIntensity = 0;
};

static NodeFeatures DummyNodeFeatures; 

class MNode : public MNodeBase {
public:
  enum class NodeKind {
    Root,
    Feature,
    FunctionRoot
  };

  MNode() = delete;
  MNode(const NodeFeatures &F) : MNodeBase(), Features(F) {}
  MNode(const MNode &N) : MNodeBase(N), Features(N.Features), Kind(N.Kind) {}
  MNode(MNode &&N) : MNodeBase(std::move(N)), Features(N.Features), Kind(N.Kind) {}

  //MNode& operator=(const MNode& Other);
  virtual ~MNode() {};

  NodeKind GetKind() const { return Kind; }
  void SetKind(NodeKind K) { Kind = K; }

private:
// move features to derived?
  NodeFeatures Features;
  NodeKind Kind;
};

class RootMNode : public MNode {
public:
  RootMNode() : MNode(DummyNodeFeatures) { SetKind(NodeKind::Root); };
  ~RootMNode() {};

  static bool classof(const MNode *N) {
    return N->GetKind() == NodeKind::Root;
  }
  static bool classof(const RootMNode *N) { return true; }
};

class FeatureMNode : public MNode {
public:
  FeatureMNode(NodeFeatures &F) : MNode(F) { SetKind(NodeKind::Feature); };
  ~FeatureMNode() {};

  static bool classof(const MNode *N) {
    return N->GetKind() == NodeKind::Feature;
  }
  static bool classof(const FeatureMNode *N) { return true; }
};

class FunctionMNode : public MNode {
public:
  FunctionMNode() : MNode(DummyNodeFeatures) { SetKind(NodeKind::FunctionRoot); };
  FunctionMNode(std::string s) : MNode(DummyNodeFeatures), Name(s) { SetKind(NodeKind::FunctionRoot); };
  ~FunctionMNode() {};

  void SetName(std::string s) { Name=s; }
  std::string GetName() { return Name; }

  static bool classof(const MNode *N) {
    return N->GetKind() == NodeKind::FunctionRoot;
  }
  static bool classof(const FunctionMNode *N) { return true; }
private:
  std::string Name;
};

class MEdge : public MEdgeBase {
public:
  MEdge() = delete;
  MEdge(MNode &N) : MEdgeBase(N) {}
  //MEdge(MEdge &E) : MEdgeBase(E) {}
  virtual ~MEdge() = 0;
};

class ControlEdge : public MEdge {
public:
  ControlEdge(MNode &N) : MEdge(N) {}
};

class FDGraph {
public:
  FDGraph() {
    auto *N = new RootMNode();
    G.addNode(*N);
    Current = Root = N;
  }
  ~FDGraph() {
    for (auto &N: G) {
      delete N;
    }
  }

  void SetCurrent(MNode *N) { Current = N; }
  MNode *GetCurrent() const { return Current; }
  MNode *GetRoot() const { return Root; }

  void addNode(MNode *N) {
    G.addNode(*N);
  }

  void connect(MNode& Src, MNode& Dst, MEdge& E) {
    G.connect(Src, Dst, E);
  }

  void dump() {
    for (auto &N: G) {
      // todo
    }
  }
private:
  MDGBase G;
  RootMNode *Root;
  MNode *Current;
};

}
#endif