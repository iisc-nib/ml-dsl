#include <iostream>
#include <stdexcept>
#include <string>
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

bool Network::CheckTypes()
{
    try
    {
        for(int32_t layerID=0 ; layerID<GetNumberOfLayers() ; ++layerID)
        {
            Layer& currentLayer = GetLayer(layerID);
            for(int32_t neuronID=0 ; neuronID<currentLayer.GetNumberOfNeurons() ; ++neuronID)
            {
                Neuron& neuron = currentLayer.GetNeuron(neuronID);
                neuron.CheckTypes();
            }
        }
    }
    catch (std::runtime_error& e)
    {
        std::cout << e.what() << std::endl;
        return false;
    }
    return true;
}

Network& Network::Create()
{
    return *(new Network);
}

void Network::Destroy(Network& network)
{
    delete &network;
}

class NetworkPrintVisitor : public NetworkVisitor
{
    std::ostream& m_ostr;
    int32_t m_indent = 0;
    void PrintSourceNeurons(Neuron& neuron)
    {
        NeuronList& sources = neuron.GetSources();
        Indent();
        m_ostr << "<sources>" << std::endl;
        m_indent++;
        for (int32_t i=0 ; i<sources.size() ; ++i)
        {
            Indent();
            m_ostr << "<id>" << sources[i]->GetLayer().GetNeuronID(*sources[i]) << "</id>\n";
        }
        m_indent--;
        Indent();
        m_ostr << "</sources>" << std::endl;
    }
    void PrintSinkNeurons(Neuron& neuron)
    {
        NeuronList& sinks = neuron.GetSinks();
        Indent();
        m_indent++;
        m_ostr << "<sinks>" << std::endl;
        for (int32_t i=0 ; i<sinks.size() ; ++i)
        {
            Indent();
            m_ostr << "<id>" << sinks[i]->GetLayer().GetNeuronID(*sinks[i]) << "</id>\n";
        }
        m_indent--;
        Indent();
        m_ostr << "</sinks>" << std::endl;
    }
    void Indent()
    {
        for (int32_t i=0 ; i<m_indent ; ++i)
            m_ostr << "\t";
    }

public:
    NetworkPrintVisitor(std::ostream& ostr)
        :m_ostr(ostr)
    { }
    virtual void Visit(Network& network)
    {
        m_ostr << "<net>" << std::endl;
        m_indent++;
        for (int32_t i=0 ; i<network.GetNumberOfLayers() ; ++i)
        {
            Layer& layer = network.GetLayer(i);
            layer.AcceptVisitor(*this);
        }
        m_indent--;
        m_ostr << "</net>" << std::endl;
    }
    virtual void Visit(Layer& layer)
    {
        Indent();
        m_ostr << "<layer>" << std::endl;
        m_indent++;
        for (int32_t i=0 ; i<layer.GetNumberOfNeurons() ; ++i)
        {
            Neuron& neuron = layer.GetNeuron(i);
            neuron.AcceptVisitor(*this);
        }
        m_indent--;
        Indent();
        m_ostr << "</layer>" << std::endl;
    }
    virtual void Visit(Neuron& neuron)
    {
        Indent();
        m_ostr << "<neuron>" << std::endl;
        m_indent++;
        
        PrintSourceNeurons(neuron);
        PrintSinkNeurons(neuron);

        Indent();
        m_ostr << "<forwardvalue>" << std::endl;
        PrintValue(neuron.GetForwardPropagationValue(), m_ostr, m_indent+1);
        Indent();
        m_ostr << "</forwardvalue>" << std::endl;
        m_indent--;
        Indent();
        m_ostr << "</neuron>" << std::endl;
    }
    virtual void Visit(InputNeuron& inputNeuron)
    {
        Indent();
        m_ostr << "<inputneuron>" << std::endl;
        m_indent++;
        PrintSinkNeurons(inputNeuron);
        m_indent--;
        Indent();
        m_ostr << "</inputneuron>" << std::endl;
    }
    virtual void Visit(OutputNeuron& outputNeuron)
    {
        Indent();
        m_ostr << "<outputneuron>" << std::endl;
        m_indent++;
        PrintSourceNeurons(outputNeuron);
        Indent();
        m_ostr << "<forwardvalue>" << std::endl;
        PrintValue(outputNeuron.GetForwardPropagationValue(), m_ostr, m_indent+1);
        Indent();
        m_ostr << "</forwardvalue>" << std::endl;
        m_indent--;
        Indent();
        m_ostr << "</outputneuron>" << std::endl;
    }
};

void PrintNetwork(Network& network, std::ostream& ostr)
{
    NetworkPrintVisitor printVisitor(ostr);
    printVisitor.Visit(network);
}
