#include <iostream>
#include <stdexcept>
#include <string>
#include "valuetype.h"
#include "value.h"
#include "neuronproperty.h"
#include "neuron.h"
#include "layer.h"
#include "network.h"

class CollectMergeableNeuronsIntoEnsemblesVisitor : public NetworkVisitor
{
public:
    CollectMergeableNeuronsIntoEnsemblesVisitor()
    { }
    virtual void Visit(Network& network)
    {
        for (int32_t i=0 ; i<network.GetNumberOfLayers() ; ++i)
        {
            Layer& layer = network.GetLayer(i);
            layer.AcceptVisitor(*this);
        }
    }
    virtual void Visit(Layer& layer)
    {
        Neuron* currentEnsembleRep = nullptr;
        Ensemble *currentEnsemble = nullptr;

        for (int32_t i=0 ; i<layer.GetNumberOfNeurons() ; ++i)
        {
            Neuron& currentNeuron = layer.GetNeuron(i);
            if (currentEnsembleRep == nullptr || !AreNeuronsMergeable(currentNeuron, *currentEnsembleRep))
            {
                currentEnsemble = &(layer.CreateNewEnsemble());
                currentEnsembleRep = &currentNeuron;
            }
            currentEnsemble->AddNeuron(currentNeuron);
        }
    }
    virtual void Visit(Neuron& neuron)
    {
    }
    virtual void Visit(InputNeuron& inputNeuron)
    {
    }
    virtual void Visit(OutputNeuron& outputNeuron)
    {
    }
};


void CollectMergeableNeuronsIntoEnsembles(Network& network)
{
    CollectMergeableNeuronsIntoEnsemblesVisitor collectVisitor;
    network.AcceptVisitor(collectVisitor);
}
