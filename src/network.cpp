#include <stdexcept>
#include "valuetype.h"
#include "value.h"
#include "neuronproperty.h"
#include "neuron.h"
#include "layer.h"
#include "network.h"

Network::~Network()
{
    for (size_t i=0 ; i<m_layers.size() ; ++i)
        delete m_layers[i];
}

Layer& Network::AddLayer(int32_t& layerID)
{
    layerID = static_cast<int32_t>(m_layers.size());
    Layer *newLayer = new Layer;
    m_layers.push_back(newLayer);
    return *newLayer;
}

void Network::ConnectLayers(int32_t sourceLayerID, int32_t sinkLayerID, std::map<int32_t, std::vector<int32_t>>& connections)
{
    Layer& sourceLayer = GetLayer(sourceLayerID);
    Layer& sinkLayer = GetLayer(sinkLayerID);

    for (int32_t sinkNeuronIndex=0 ; sinkNeuronIndex<sinkLayer.GetNumberOfNeurons() ; ++sinkNeuronIndex)
    {
        std::map<int32_t, std::vector<int32_t>>::iterator iter = connections.find(sinkNeuronIndex);
        if (iter == connections.end())
        {
            throw std::runtime_error("Connections for all neurons in sink layer not specified by the connection map");
        }
        Neuron& sinkNeuron = sinkLayer.GetNeuron(sinkNeuronIndex);
        std::vector<int32_t>& sourceNeuronIndices = iter->second;
        for (size_t i=0; i<sourceNeuronIndices.size() ; ++i)
        {
            ConnectNeurons(sourceLayer.GetNeuron(sourceNeuronIndices[i]), sinkNeuron);
        }
    }
}

Network& Network::Create()
{
    return *(new Network);
}

void Network::Destroy(Network& network)
{
    delete &network;
}
