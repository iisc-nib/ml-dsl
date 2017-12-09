#ifndef _NETWORK_H_
#define _NETWORK_H_

#include <vector>
#include <map>

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
    void ConnectLayers(int32_t sourceLayer, int32_t sinkLayer, std::map<int32_t, std::vector<int32_t>>& connections);

    static Network& Create();
    static void Destroy(Network& layer);
};

#endif // _NETWORK_H_
