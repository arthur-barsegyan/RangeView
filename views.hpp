#import <vector>
#import <functional>
#import <iostream>

namespace view {

template<typename T>
class RangeView {
public:
	typedef std::function< std::vector<T>(std::vector<T>&) > Action;
	typedef std::vector<Action> Actions;

	RangeView(std::function< std::vector<T>(std::vector<T>&) > func) {
		actions.push_back(func);
	}

	// RangeView operator|(RangeView &another) {

	// }

	template<class Y>
	friend RangeView<Y> operator|(std::vector<Y> &v, RangeView<Y> rv);

	template<typename Y, typename F>
	friend std::vector<F> operator|(RangeView<Y> rv, std::function< std::vector<F>(std::vector<Y>&) > termPred);
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

	int currentAction = 0;
	std::vector<T> *v;
	Actions actions;	
};

template<typename T>
RangeView<T> operator|(std::vector<T> &vec, RangeView<T> rv) {
	std::cout << "Create RangeView stream" << std::endl;
	rv.setCollection(&vec);

	return rv;
}

template<typename T, typename F>
std::vector<F> operator|(RangeView<T> rv, std::function< std::vector<F>(std::vector<T>&) > termPred) {
	std::vector<T> &result = rv.getCollection(); 
	for (auto &act : rv.getActions()) {
		result = act(result);
	}

	return termPred(result);
}

template<typename T, typename F>
RangeView<T> remove_if(std::function< F(T) > &pred) {
	std::function< std::vector<T>(std::vector<T>&) > remove_if_func = [pred](std::vector<T> &v) {
		for (size_t i = 0; i < v.size();) {
			if (pred(v[i])) {
				v.erase(v.begin() + i);
			} else {
				i++;
			}
		}

		return v;
	};

	return RangeView<T>(remove_if_func);
}

template<typename T, typename F>
std::function< std::vector<F>(std::vector<T>&) > transform(std::function< F(T) > &pred) {
	std::function< std::vector<F>(std::vector<T>&) > transform_func = [pred](std::vector<T> &v) {
		std::vector<F> result;
		for (size_t i = 0; i < v.size(); i++) {
			result.push_back(pred(v[i]));
		}

		return result;
	};

	return transform_func;
}

}