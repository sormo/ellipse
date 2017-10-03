#pragma once
#include <vector>
#include <memory>

class AnimationBody;

struct AnimationBodyNode
{
public:
	AnimationBodyNode(const std::vector<std::unique_ptr<AnimationBody>> & bodies);

	AnimationBody * GetParent(AnimationBody * node);
	std::vector<AnimationBody*> GetChilds(AnimationBody * node);

private:
	AnimationBodyNode(AnimationBody * value);
	AnimationBodyNode * DFS(AnimationBodyNode * node, AnimationBody * body);

	std::vector<std::shared_ptr<AnimationBodyNode>> m_childs;
	std::shared_ptr<AnimationBodyNode> m_parent;

	AnimationBody * m_value;
};
