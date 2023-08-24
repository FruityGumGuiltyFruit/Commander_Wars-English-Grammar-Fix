#include "ai/neuralnetwork/neural/neuralnetwork.h"
#include "ai/neuralnetwork/neural/layer.h"

#include "coreengine/memorymanagement.h"
#include "coreengine/interpreter.h"

NeuralNetwork::NeuralNetwork(double maxWeight)
    : m_maxWeight(maxWeight)
{
    Interpreter::setCppOwnerShip(this);
}

void NeuralNetwork::autogenerate(bool randomize)
{
	connectComplete();
	if(randomize)
    {
		randomizeAllWeights();
    }	
}

void NeuralNetwork::addLayer(QMap<QString, double> & parameters)
{
    m_configuration.append(parameters);
    m_layers.push_back(MemoryManagement::create<Layer>(m_layers.size(), this, parameters));
}

void NeuralNetwork::extend(quint32 count, bool randomize)
{
    for(qint32 i = 0; i < m_layers.size(); ++i)
    {
        m_configuration[i][Layer::LAYER_PARAMETER_SIZE] += count;
        m_layers[i]->extend(count, randomize);
    }
}

void NeuralNetwork::clean()
{
    for(spLayer & l : m_layers)
    {
        l->clean();
    }
}

void NeuralNetwork::setInput(const std::vector<double> & in)
{
	clean();
    for(qint32 i = 0; i < in.size(); ++i)
    {
        double value = in[i];
        m_layers[0]->setAccumulated(i, value);
    }
}

void NeuralNetwork::trigger()
{
    for (spLayer & l : m_layers)
    {
        l->trigger();
    }
}

std::vector<double> NeuralNetwork::output()
{
    auto ret = (m_layers.back())->output();
    if (m_disabled)
    {
        for (auto & item : ret)
        {
            item = 0;
        }
    }
    return ret;
}

double NeuralNetwork::output(qint32 index)
{
    if (m_disabled)
    {
        return 0;
    }
    else
    {
        return m_layers.back()->output()[index];
    }

}

void NeuralNetwork::connectComplete()
{
    for(qint32 i_layer = 0; i_layer < m_layers.size() - 1; ++i_layer)
    {
        m_layers[i_layer]->connectComplete(m_layers[i_layer + 1].get());
    }
}

void NeuralNetwork::randomizeAllWeights()
{
    for(qint32 i_layer = 0; i_layer < m_layers.size() - 1; ++i_layer)
    {
        m_layers[i_layer]->randomizeAllWeights(m_maxWeight);
    }
}

bool NeuralNetwork::getDisabled() const
{
    return m_disabled;
}

void NeuralNetwork::setDisabled(bool newDisabled)
{
    m_disabled = newDisabled;
}

void NeuralNetwork::mutateAllWeights(double mutationChance, double mutationRate)
{
    for(qint32 i_layer = 0; i_layer < m_layers.size() - 1; ++i_layer)
    {
        m_layers[i_layer]->mutateAllWeights(mutationChance, m_maxWeight, mutationRate);
    }
}

const QString &NeuralNetwork::getNetworkName() const
{
    return m_networkName;
}

void NeuralNetwork::setNetworkName(const QString &newNetworkName)
{
    m_networkName = newNetworkName;
}

QString NeuralNetwork::toString()
{
    QString s = "NeuralNetwork";
    s.push_back('\n');
    for(spLayer & l : m_layers)
    {
        s += l->toString();
    }
    return s;
}
std::vector<double> NeuralNetwork::predict(const std::vector<double>& in)
{
	setInput(in);
	trigger();
	return output();
}

void NeuralNetwork::serializeObject(QDataStream& pStream) const
{
    pStream << getVersion();
    pStream << m_fitness;
    pStream << m_configuration;
    pStream << m_maxWeight;
    pStream << static_cast<qint32>(m_layers.size());
    for (const auto & layer : m_layers)
    {
        layer->serializeObject(pStream);
    }
}

void NeuralNetwork::deserializeObject(QDataStream& pStream)
{
    qint32 version = 0;
    pStream >> version;
    pStream >> m_fitness;
    pStream >> m_configuration;
    pStream >> m_maxWeight;
    qint32 size = 0;
    pStream >> size;
    for (qint32 i = 0; i < size; ++i)
    {
        spLayer layer = MemoryManagement::create<Layer>(i, this, m_configuration[i]);
        if (i > 0)
        {
            layer->setPreviousLayer(m_layers[i - 1].get());
        }
        layer->deserializeObject(pStream);
        m_layers.push_back(layer);
    }
}
