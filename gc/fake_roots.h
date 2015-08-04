/************************************************************************************//**
		* File: fake_roots.h
		* Description: realize fake_roots structure in th AVL-tree form.
			It can be realized in any other (i.e. hash) data structure.
		* Detaited: fake root --- is a garbage collection root that is not automatic,
			i.e. created outside program stack and static memory,
			and have to be manually registered and deregistered by user.
			Thous roots represent references from unmanaged objects to the managed.
		* Unclear: perhapse we need some way to the following:
			since that reference was created till it be registered
			garbage collection cannot executes.
*****************************************************************************************/

#pragma once
#include "go.h"

/// represents node structure
struct node {
	void * addr;
	unsigned char height;
	node * left;
	node * right;
};

node * fake_roots = NULL;

void mark_node (node * p) {
	if (!p) return;
//	mark(p);
	go(get_next_obj(p->addr));
	mark_node(p->left);
	mark_node(p->right);
};

void mark_fake_roots () {
	mark_node(fake_roots);
};

unsigned char height (node * p) {
	return p ? p->height : 0;
};

/// return balance factor
int bfactor (node * p) {
	return height(p->right) - height(p->left);
}

/// fix height of node p
/// NB: all chields p must have right height
void fixheight (node * p) {
	unsigned char hl = height(p->left);
	unsigned char hr = height(p->right);
	p->height = (hl > hr ? hl : hr) + 1;
};

/// right turn around p
node * rotateright (node * p) {
	node * q = p->left;
	p->left = q->right;
	q->right = p;
	fixheight(p);
	fixheight(q);
	return q;
};

/// left turn around q
node * rotateleft (node * q) {
	node * p = q->right;
	q->right = p->left;
	p->left = q;
	fixheight(q);
	fixheight(p);
	return p;
};

/// balance tree p
node * balance (node * p) {
	fixheight(p);
	if (bfactor(p) == 2) {
		if (bfactor(p->right) < 0) {
			p->right = rotateright(p->right);
		}
		return rotateleft(p);
	}
	if (bfactor(p) == -2) {
		if (bfactor(p->left) > 0) {
			p->left = rotateleft(p->left);
		}
		return rotateright(p);
	}
	return p;
};

/// insert address k in tree p
node * insert (node * p, void * k) {
	if (!p) {
		node * new_node = (node *) malloc(sizeof(node));
		//transfer_to_automatic_objects(new_node);
		new_node->addr = k;
		new_node->right = new_node->left = 0;
		new_node->height = 1;
		return new_node;
	}
	if (k < p->addr) {
		p->left = insert(p->left, k);
	} else {
		p->right = insert(p->right, k);
	}
	return balance(p);
}

/// find node with minimum address value in tree p 
node * findmin (node * p) {
    return p->left ? findmin(p->left) : p;
};

/// delete a node with minimum address value frome tree p
node * removemin (node * p) {
	if (p->left == 0) {
		return p->right;
	}
	p->left = removemin(p->left);
	return balance(p);
};

/// delete address k frome node p
node * remove (node * p, void * k) {
	if (!p) return 0;
	if (k < p->addr) {
		p->left = remove(p->left, k);
	} else if (k > p->addr) {
		p->right = remove(p->right, k);	
	} else {
		node * q = p->left;
		node * r = p->right;
		// delete p;
		if (!r) return q;
		node * min = findmin(r);
		min->right = removemin(r);
		min->left = q;
		return balance(min);
	}
	return balance(p);
};

/**
* @function register_object
* @brief 
* @params p --- is a pointer to be registered as a fake root
* @return nothing
*/
/* TODO: return success or not */
void register_object (void * p) {
	fake_roots = insert(fake_roots, p);
};

/**
* @function unregister_object
* @brief 
* @params p --- is a pointer to be unregistered as a fake root;
*	that pointer have to be manually registered earlier
* @return nothing
*/
/* TODO: needs some check that pointer is valid and contains in
*	and return success or error code
*/
void unregister_object (void * p) {
	fake_roots = remove(fake_roots, p);
};