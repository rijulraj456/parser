// LICENSE
// This soNodeTware is free for use and redistribution while including this
// license notice, unless:
// 1. is used for commercial or non-personal purposes, or
// 2. used for a product which includes or associated with a blockchain or other
// decentralized database technology, or
// 3. used for a product which includes or associated with the issuance or use
// of cryptographic or electronic currencies/coins/tokens.
// On all of the mentioned cases, an explicit and written permission is required
// from the Author (Ohad Asor).
// Contact ohad@idni.org for requesting a permission. This license may be
// modified over time by the Author.
#ifndef __IDNI__PARSER__FOREST_H__
#define __IDNI__PARSER__FOREST_H__
namespace idni {

#ifdef DEBUG
#define DBG(x) x
#else
#define DBG(x)
#endif

template <typename NodeT>
struct forest {
	typedef NodeT node;
	typedef std::vector<node> nodes;
	typedef std::set<nodes> nodes_set;
	typedef std::map<node, nodes_set> node_graph;
	typedef std::pair<size_t, size_t> edge;
	typedef std::vector<edge> edges;
	typedef std::pair<nodes, edges> nodes_and_edges;
	node_graph g;
	void clear() { g.clear(); }
	bool contains(const node& n) { return g.find(n) != g.end(); }
	nodes_set& operator[](const node& p)             { return g[p]; }
	const nodes_set& operator[](const node& p) const { return g[p]; }	

	size_t count_trees(const node& root);
	nodes_and_edges get_nodes_and_edges() const;

	template <typename cb_enter_t, typename cb_exit_t,
		typename cb_revisit_t, typename cb_ambig_t>
	bool traverse(const node &root, cb_enter_t cb_enter, cb_exit_t cb_exit,
		cb_revisit_t cb_revisit, cb_ambig_t cb_ambig) const;
	// traverse methods with default parameters
	template <typename cb_enter_t>
	bool traverse(const node &root, cb_enter_t cb_enter) const {
		return traverse(root, cb_enter, ([](const auto&,const auto&){}),
			([](const auto&) { return true; }),
			([](const auto&, auto&) { return nodes_set{}; }));
	}
	template <typename cb_enter_t, typename cb_exit_t>
	bool traverse(const node &root, cb_enter_t cb_enter, cb_exit_t cb_exit)
		const
	{
		return traverse(root, cb_enter, cb_exit,
			([](const auto&) { return true; }),
			([](const auto&, auto&) { return nodes_set{}; }));
	}
	template <typename cb_enter_t, typename cb_exit_t,typename cb_revisit_t>
	bool traverse(const node &root, cb_enter_t cb_enter, cb_exit_t cb_exit,
		cb_revisit_t cb_revisit) const
	{
		return traverse(root, cb_enter, cb_exit, cb_revisit,
			([](const auto&, auto&) { return nodes_set{}; }));
	}
private:
	template <typename cb_enter_t, typename cb_exit_t,
		typename cb_revisit_t, typename cb_ambig_t>
	bool _traverse(const node &root, std::set<node> &done,
		cb_enter_t cb_enter, cb_exit_t cb_exit,
		cb_revisit_t cb_revisit, cb_ambig_t cb_ambig) const;
};

template <typename NodeT>
typename forest<NodeT>::nodes_and_edges forest<NodeT>::get_nodes_and_edges()
	const
{
	std::map<node, size_t> nid;
	std::map<size_t, node> ns;
	nodes n;
	edges es;
	size_t id = 0;
	for (auto& it : g) {
		nid[it.first] = id;
		// skip ids for one child ambig node
		id += it.second.size() == 1 ? 0 : it.second.size(); // ambig node ids;
		//DBG(assert(it.second.size()!= 0));
		id++;
	}
	for (auto& it : g) {
		ns[nid[it.first]] = it.first;
		size_t p = 0;
		for (auto& pack : it.second) {
			if (it.second.size() > 1) {  //skipping if only one ambigous node, an optimization
				++p;
				ns[nid[it.first] + p] = it.first;
				es.emplace_back(nid[it.first], nid[it.first]+p);
			}
			for (auto& nn : pack) {
				if (nid.find(nn) == nid.end()) nid[nn] = id++; // for terminals, not seen before
				ns[nid[nn]] = nn;
				es.emplace_back(nid[it.first] + p, nid[nn]);
			}
		}
	}
	n.resize(id);
	for (auto& p : ns) n[p.first] = p.second;
	return nodes_and_edges{ n, es };
}

template <typename NodeT>
template <typename cb_enter_t, typename cb_exit_t,
	typename cb_revisit_t, typename cb_ambig_t>
bool forest<NodeT>::traverse(const node &root, cb_enter_t cb_enter,
	cb_exit_t cb_exit, cb_revisit_t cb_revisit, cb_ambig_t cb_ambig) const
{
	std::set<node> done;
	return _traverse(root, done, cb_enter, cb_exit, cb_revisit, cb_ambig);
}

template <typename NodeT>
template <typename cb_enter_t, typename cb_exit_t,
	typename cb_revisit_t, typename cb_ambig_t>
bool forest<NodeT>::_traverse(const node &root, std::set<node> &done,
	cb_enter_t cb_enter, cb_exit_t cb_exit,
	cb_revisit_t cb_revisit, cb_ambig_t cb_ambig) const
{
	bool ret = true;
	nodes_set pack;
	if (root.first.nt()) {
		auto it = g.find(root);
		if (it == g.end()) return false;
		pack = it->second;
	}
	cb_enter(root);
	done.insert(root);

	nodes_set choosen_pack = pack.size() > 1
		? cb_ambig(root, pack) : pack; 
	for (auto& nodes : choosen_pack)
		for (auto& chd : nodes)
			if (!done.count(chd) || cb_revisit(chd)) 
				ret &= _traverse(chd, done, cb_enter, cb_exit,
					cb_revisit, cb_ambig);
	cb_exit(root, choosen_pack);
	return ret;
}

template <typename NodeT>
size_t forest<NodeT>::count_trees(const node& root) {
	size_t count = 1;
	auto cb_enter = [](const auto&) {};
	auto cb_exit =  [](const auto&, const auto&) {};
	auto cb_keep_ambig = [&count](const node&, auto& ambset) {
		count *= ambset.size();
		return ambset;
	};
	auto cb_revisit =  [](const auto&) { return false; }; // revisit
	traverse(root, cb_enter, cb_exit, cb_revisit, cb_keep_ambig);
	return count; 
}

} // idni namespace
#endif // __IDNI__PARSER__FOREST_H__
