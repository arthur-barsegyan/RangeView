#import <vector>
#import <functional>
#import <algorithm>
#import <numeric>
#import <iostream>

template <class TView>
auto accumulate(TView view) {
	auto collection = view.toVector();
	using type = std::decay_t<decltype(collection[0])>;

	return std::accumulate(collection.begin(), collection.end(), type());
}

namespace view {

	template<typename T>
	class RangeView;

	template<typename Func>
	class TerminationOp {
	public:
		TerminationOp(Func &_f) : f(_f) {}

		template<typename T>
		auto apply(const std::vector<T> &v, RangeView<T> &rv) {
			return f(v, rv);
		}
	private:
		Func f;
	};

	template<typename Func>
	class LazyTerminationOp : public TerminationOp<Func> {
	public:
		LazyTerminationOp(Func &_f) : TerminationOp<Func>(_f) {}
	};

	class EndlessSequenceException {};
	
	template<typename T>
	class RangeView {
	public:
		typedef std::function< std::vector<T>(std::vector<T>&, RangeView<T>&) > Action;
		typedef std::vector<Action> Actions;

		RangeView(Action generator) : extCollection(std::vector<T>()), seqGenerator(generator), hasGenerator(true), endless(true) {}
		RangeView(const std::vector<T> &v) : extCollection(v) {}

		// TODO: Maybe make it private?
		void setCount(int _count) {
			count = _count;
			endless = false;
		}

		int getCount() {
			return count;
		}

		bool isEndless() {
            return endless;
		}

		std::vector<T> toVector();

		// TODO: Maybe make it private?
        void addAction(Action action) {
            actions.push_back(action);
        }


		template<class Y, class Z>
		friend RangeView<Y> operator|(std::vector<Y> &v, Z func);

		template<class Y, class Z>
		friend RangeView<Y> operator|(RangeView<Y> rv, Z func);

		template<typename Y, typename Z>
		friend auto operator|(RangeView<Y> rv, TerminationOp<Z> termOp);

        template<typename Y, typename Z>
        friend auto operator|(RangeView<Y> rv, LazyTerminationOp<Z> termOp);
	private:
		const std::vector<T> &getCollection() {
			return extCollection;
		}

		const std::vector<T> &extCollection;

		Actions &getActions() {
			return actions;
		}

		int count = 0;
		bool endless = false;
        bool hasGenerator = false;
        Action seqGenerator;
		Actions actions;
	};

	template<typename T>
	std::vector<T> RangeView<T>::toVector() {
        std::vector<T> result;
        if (hasGenerator) {
        	if (endless) {
        		throw EndlessSequenceException();
        	}

            seqGenerator(result, *this);
        } else {
        	result.clear();
        	result.assign(this->getCollection().begin(), this->getCollection().end());
        }

		for (auto &act : this->getActions()) {
			act(result, *this);
		}

		return result;
	}

	template<typename T, typename F>
	RangeView<T> operator|(std::vector<T> &vec, F func) {
		RangeView<T> rv = RangeView<T>(vec);
		rv.addAction(std::function< std::vector<T>(std::vector<T>&, RangeView<T>&) >(func));

		return rv;
	}

	template<typename T, typename F>
	RangeView<T> operator|(RangeView<T> rv, F func) {
		rv.addAction(std::function< std::vector<T>(std::vector<T>&, RangeView<T>&) >(func));
		return rv;
	}

    template<typename T, typename F>
    auto operator|(RangeView<T> rv, TerminationOp<F> termOp) {
    	auto temp = rv.toVector();

    	using type = std::decay_t<decltype(termOp.apply(temp, rv))>;
        auto newData = termOp.apply(temp, rv);

        return RangeView<std::decay_t<decltype(newData[0])>>(newData);
    }

    template<typename T, typename F>
    auto operator|(RangeView<T> rv, LazyTerminationOp<F> termOp) {
		termOp.apply(std::vector<T>(), rv);
		return rv;
    }

	RangeView<int> ints(int n) {
		auto int_generator_func = [n](auto &v, auto &rv) {
			for (size_t i = n, count = 0; count < rv.getCount() || rv.isEndless(); i++, count++) {
				v.push_back(i);
			}

			return v;
		};

		RangeView<int> rv = RangeView<int>(
                std::function<std::vector<int>(std::vector<int>&, RangeView<int>&)>(int_generator_func));

		return rv;
	}

	auto take(int n) {
		auto take_func = [n](auto &v, auto &rv) {
            std::cout << n << std::endl;
			rv.setCount(n);
			return v;
		};

        return LazyTerminationOp<decltype(take_func)>(take_func);
	}

	auto reverse() {
		auto reverse_func = [](auto &v, auto &rv) {
			std::reverse(v.begin(), v.end());
			return v;
		};

		return reverse_func;
	}

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
			// TODO: && i < rv.getCount()
			for (size_t i = 0; i < v.size() ; i++) {
				result.push_back(pred(v[i]));
			}

			return result;
		};

		return TerminationOp<decltype(transform_func)>(transform_func);
	}

}