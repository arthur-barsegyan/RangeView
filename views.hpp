#import <vector>
#import <functional>
#import <algorithm>
#import <iostream>

namespace view {

template<typename T>
class RangeView;

template<typename Func>
class TerminationOp {
public:
	TerminationOp(Func _f) : f(_f) {}

	template<typename T>
	auto apply(std::vector<T> &v, RangeView<T> &rv) {
		return f(v, rv);
	}
private:
	Func &f;
};

template<typename T>
class RangeView {
public:
	typedef std::function< std::vector<T>(std::vector<T>&, RangeView<T>&) > Action;
	typedef std::vector<Action> Actions;

	RangeView(Action func) {
		actions.push_back(func);
	}

	void setCount(size_t _count) {
		count = _count;
	}

	size_t getCount() {
		return count;
	}

	template<class Y, class Z>
	friend RangeView<Y> operator|(std::vector<Y> &v, Z func);

	template<class Y, class Z>
	friend RangeView<Y> operator|(RangeView<Y> rv, Z func);

	template<typename Y, typename Z>
	friend auto operator|(RangeView<Y> rv, TerminationOp<Z> func);

private:
	void setCollection(std::vector<T> *_v) {
		v = _v;
	} 

	std::vector<T> &getCollection() {
		return *v;
	}

	Actions &getActions() {
		return actions;
	}

	void addAction(Action action) {
		actions.push_back(action);
	}

	size_t count = 0;
	
	std::vector<T> *v;
	Actions actions;	
};

template<typename T, typename F>
RangeView<T> operator|(std::vector<T> &vec, F func) {
	std::cout << "Create RangeView stream" << std::endl;
	RangeView<T> rv = RangeView<T>(std::function< std::vector<T>(std::vector<T>&, RangeView<T>&) >(func));
	rv.setCollection(&vec);

	return rv;
}

template<typename T, typename F>
RangeView<T> operator|(RangeView<T> rv, F func) {
	rv.addAction(std::function< std::vector<T>(std::vector<T>&, RangeView<T>&) >(func));
	return rv;
}

template<typename T, typename F>
auto operator|(RangeView<T> rv, TerminationOp<F> func) {
	std::vector<T> &result = rv.getCollection(); 
	for (auto &act : rv.getActions()) {
		act(result, rv);
	}

	return func.apply(result, rv);
}

auto ints(int n) {
	auto int_generator_func = [n](auto &v, auto &rv) {
		for (size_t i = n; i < rv.getCount(); i++) {
			v.push_back(i);
		}

		return v;
	};

	return int_generator_func;
}

auto take(int n) {
	auto take_func = [n](auto &v, auto &rv) {
		rv.setCount(n);
		return v;
	};

	return take_func;
}

auto reverse() {
	auto reverse_func = [](auto &v, auto &rv) {
		std::reverse(v.begin(), v.end());
		return v;
	};

	return reverse_func;
}

// template <class TView>
// auto accumulate(TView view) {

// }

template<typename F>
auto remove_if(F &&pred) {
	auto remove_if_func = [pred](auto &v, auto &rv) {
		for (size_t i = 0; i < v.size();) {
			if (pred(v[i])) {
				v.erase(v.begin() + i);
			} else {
				i++;
			}
		}

		return v;
	};

	return remove_if_func;
}

template<typename F>
auto transform(F &&pred) {
	auto transform_func = [pred](auto &v, auto &rv) {
		using type = std::decay_t<decltype(pred(v[0]))>;

		std::vector<type> result;
		for (size_t i = 0; i < v.size(); i++) {
			result.push_back(pred(v[i]));
		}

		return result;
	};

	return TerminationOp<decltype(transform_func)>(transform_func);
}

}