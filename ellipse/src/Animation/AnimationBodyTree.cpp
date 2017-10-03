#include "AnimationBodyTree.h"
#include <cassert>
#include "AnimationBody.h"

AnimationBodyNode::AnimationBodyNode(AnimationBody * value)
	: m_value(value)
{

}

AnimationBodyNode::AnimationBodyNode(const std::vector<std::unique_ptr<AnimationBody>> & bodies)
{
	std::vector<std::pair<AnimationBody*, std::shared_ptr<AnimationBodyNode>>> nodes;
	for (auto & body : bodies)
		nodes.push_back({ body.get(), std::shared_ptr<AnimationBodyNode>(new AnimationBodyNode(body.get())) });

	for (auto & node : nodes)
	{
		std::lock_guard<const simmulation::CelestialBody> lock(*node.first->GetCelestialBody().get());

		const auto & parentCelestial = node.first->GetCelestialBody()->GetParent();
		if (!parentCelestial)
		{
			m_childs.push_back(node.second);
			continue;
		}

		std::shared_ptr<AnimationBodyNode> parent;
		for (const auto & searchParent : nodes)
		{
			if (searchParent.first->GetCelestialBody() == parentCelestial)
			{
				parent = searchParent.second;
				break;
			}
		}
		assert(parent);

		node.second->m_parent = parent;
		parent->m_childs.push_back(node.second);
	}
}

AnimationBody * AnimationBodyNode::GetParent(AnimationBody * body)
{
	AnimationBodyNode * node = DFS(this, body);
	assert(node);
	if (node->m_parent)
		return node->m_parent->m_value;
	return nullptr;
}

AnimationBodyNode * AnimationBodyNode::DFS(AnimationBodyNode * node, AnimationBody * body)
{
	if (node->m_value == body)
		return node;

	if (node->m_childs.empty())	
	return nullptr;

	for (auto & child : node->m_childs)
	{
		auto node = DFS(child.get(), body);
		if (node)
			return node;
	}
	return nullptr;
}

std::vector<AnimationBody*> AnimationBodyNode::GetChilds(AnimationBody * body)
{
	AnimationBodyNode * node = DFS(this, body);
	assert(node);

	std::vector<AnimationBody*> ret;
	for (auto & childs : node->m_childs)
		ret.push_back(childs->m_value);
	return ret;
}
