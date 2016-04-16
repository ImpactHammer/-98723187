#pragma once
#include <iostream>
#include <ctime>

template<class E> class skiplist
{
private:

	class node
	{
	private:

	public:

		E* data;
		int height;
		node** next;

		node(E*, int, int, node**);
		~node();
	};

	node* header;
	int currentHeight;
	int maxHeight;
	float incrChance = 0.5;

	bool throwCoin(float chance)
	{
		return !(rand() % (int)(1.0 / chance));
	}

public:
	skiplist();

	bool add(E);
	bool contains(E);
	bool remove(E);

};

template<class E> skiplist<E>::skiplist()
{
	header = new node(NULL, 0, 0, NULL);
	currentHeight = 0;
	maxHeight = 15;

	srand(time(NULL));
}

template<class E> skiplist<E>::node::node(E* data, int height, int nextCapacity, node** next)
{
	this->data = data;
	this->height = height;
	this->next = new node*[height + 1];

	for (int i = 0; i <= this->height; i++)
	{
		this->next[i] = NULL;
	};

	if (next != NULL)
	{
		for (int i = 0; i <= this->height, i <= nextCapacity; i++)
		{
			this->next[i] = next[i];
		};
	}
}

template<class E> skiplist<E>::node::~node()
{
	delete data, next;
}

template<class E> bool skiplist<E>::add(E e)
{
	int height = 0;
	while ((throwCoin(incrChance)) && (height < maxHeight))
	{
		height++;
	};

	node* newHeader;
	if (height > currentHeight)
	{
		newHeader = new node(NULL, height, header->height, header->next);
	}
	else
	{
		newHeader = header;
	}

	node** prevNodes = new node*[height + 1];
	node** nextNodes = new node*[height + 1];

	for (int i = 0; i <= height; i++)
	{
		prevNodes[i] = newHeader;
		nextNodes[i] = NULL;
	};

	node* current = newHeader;
	node* next = newHeader->next[currentHeight];

	for (int level = currentHeight; level >= 0; level--)
	{
		next = current->next[level];
		while (next != NULL)
		{
			if (*(next->data) == e)
			{
				return false;
			}
			else if (*(next->data) > e)
			{
				next = current;
				break;
			}
			current = next;
			next = next->next[level];
		}
		if (level <= height)
		{
			prevNodes[level] = current;
			nextNodes[level] = current->next[level];
		}
	}

	if (height > currentHeight)
	{
		header = newHeader;
		currentHeight = height;
	}

	node* newNode = new node(new E(e), height, height, nextNodes);
	for (int i = 0; i <= height; i++)
	{
		prevNodes[i]->next[i] = newNode;
	}

	return true;
}

template<class E> bool skiplist<E>::contains(E e)
{
	node* current = header;
	node* next = header->next[currentHeight];
	for (int level = currentHeight; level >= 0; level--)
	{
		next = current->next[level];
		while (next != NULL)
		{
			if (*(next->data) == e)
			{
				return true;
			}
			else if (*(next->data) > e)
			{
				next = current;
				break;
			}
			current = next;
			next = next->next[level];
		}
	}
	return false;
}

template<class E> bool skiplist<E>::remove(E e)
{
	node** prevNodes = new node*[currentHeight + 1];
	node* current = header;
	node* next = header->next[currentHeight];
	node* found = NULL;
	for (int i = 0; i <= currentHeight; i++)
	{
		prevNodes[i] = header;
	};

	for (int level = currentHeight; level >= 0; level--)
	{
		next = current->next[level];
		while (next != NULL)
		{
			if (*(next->data) == e)
			{
				if (found == NULL)
				{
					found = next;
				}
				break;
			}
			else if (*(next->data) > e)
			{
				next = current;
				break;
			}
			current = next;
			next = next->next[level];
		}
		prevNodes[level] = current;
	}
	if (found == NULL)
	{
		return false;
	}
	for (int i = 0; i <= found->height; i++)
	{
		prevNodes[i]->next[i] = found->next[i];
	}
	delete found;
	return true;
}
