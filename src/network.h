#ifndef _NETWORK_H_
#define _NETWORK_H_

#include <vector>
#include <map>
#include "networkvisitor.h"

class Layer;

class Network
{
    std::vector<Layer*> m_layers;
public:
    Network() { }
    ~Network();
    Layer& AddLayer(int32_t& layerID);
    // TODO Should we support the following API? What happens to connections when a layer is inserted?
    // Layer& InsertLayer(int32_t index);
    Layer& GetLayer(int32_t index) { return *m_layers[index]; }
    int32_t GetNumberOfLayers() { return static_cast<int32_t>(m_layers.size()); }
    // TODO should we only take sourceLayer and assume sink is the next layer?
    void ConnectLayers(int32_t sourceLayer, int32_t sinkLayer, std::map<int32_t, std::vector<int32_t>>& connections);
    bool CheckTypes();

    virtual void AcceptVisitor(NetworkVisitor& visitor) { visitor.Visit(*this); }

    static Network& Create();
    static void Destroy(Network& layer);
};

void PrintNetwork(Network& network, std::ostream& ostr);

#endif // _NETWORK_H_
