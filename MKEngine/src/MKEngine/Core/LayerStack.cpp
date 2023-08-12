#include "mkpch.h"
#include "MKEngine/Core/LayerStack.h"

namespace MKEngine {

	LayerStack::~LayerStack()
	{
		for (Layer* layer : m_layers)
		{
			layer->OnDetach();
			delete layer;
		}
	}

	void LayerStack::PushLayer(Layer* layer)
	{
		m_layers.emplace(m_layers.begin() + m_layerInsertIndex, layer);
		m_layerInsertIndex++;
	}

	void LayerStack::PushOverlay(Layer* overlay)
	{
		m_layers.emplace_back(overlay);
	}

	void LayerStack::PopLayer(Layer* layer)
	{
		if (const auto it = std::find(m_layers.begin(), m_layers.begin() + m_layerInsertIndex, layer); it != m_layers.begin() + m_layerInsertIndex)
		{
			layer->OnDetach();
			m_layers.erase(it);
			m_layerInsertIndex--;
		}
	}

	void LayerStack::PopOverlay(Layer* overlay)
	{
		if (const auto it = std::find(m_layers.begin() + m_layerInsertIndex, m_layers.end(), overlay); it != m_layers.end())
		{
			overlay->OnDetach();
			m_layers.erase(it);
		}
	}

}