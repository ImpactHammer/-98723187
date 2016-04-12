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

};

template<class E> skiplist<E>::skiplist()
{
	header = new skiplist::node(NULL, 0, 0, NULL);
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

template<class E> bool skiplist<E>::add(E e)
{
	int height = 0;
	while ((throwCoin(incrChance)) && (height < maxHeight))
	{
		height++;
	};
	node** prevNodes = new node*[height + 1];
	node** nextNodes = new node*[height + 1];

	if (height > currentHeight)
	{
		node* newHeader = new node(NULL, height, header->height, header->next);
		header = newHeader;
		currentHeight = height;
	}

	node* current = header;
	node* next = header->next[currentHeight];

	for (int i = 0; i <= height; i++)
	{
		prevNodes[i] = header;
		nextNodes[i] = NULL;
	};

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
				if (level <= height)
				{
					prevNodes[level] = current;
					nextNodes[level] = next;
				}
				next = current;
				break;
			}
			current = next;
			next = next->next[level];
		}
	}

	for (int i = height; i >= 0; i--)
	{
		if (prevNodes[i] != header)
		{
			cout << (prevNodes[i]);
		}
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