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

	template <typename Func>
	class Op {
	public:
		Op(Func &_f) : f(_f) {}

		template <typename T>
		auto operator()(const std::vector<T> &v, RangeView<T> &rv) {
			return f(v, rv);
		}

		Func getFunc() {
			return f;
		}
	private:
		Func f;
	};

	template<typename Func>
	class TerminationOp {
	public:
		TerminationOp(Func &_f) : f(_f) {}

		template<typename T>
		auto apply(const std::vector<T> &v, RangeView<T> &rv) {
			return f(v, rv);
		}

		Func getFunc() {
			return f;
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

		RangeView() : extCollection(std::vector<T>()) {}
		RangeView(Action generator) : extCollection(std::vector<T>()), seqGenerator(generator), hasGenerator(true), endless(true) {}
		RangeView(std::vector<T> &v) : extCollection(v) {}

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

        std::vector<T> &getResultVec() {
            return result;
        }

		std::vector<T> toVector();

		// TODO: Maybe make it private?
        void addAction(Action action) {
            actions.push_back(action);
        }

        void setExtCollection(const std::vector<T> &collection) {
        	extCollection.clear();
        	extCollection.insert(extCollection.begin(), collection.begin(), collection.end());
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

		std::vector<T> extCollection;
        std::vector<T> result;

		Actions &getActions() {
			return actions;
		}

		int count = -1;
		bool endless = false;
        bool hasGenerator = false;
        Action seqGenerator;
		Actions actions;
	};

	template<typename T, typename R>
	struct RangeViewBridge {
		RangeViewBridge(RangeView<T> _old, RangeView<R> _new, std::function< std::vector<R>(std::vector<T>&, RangeView<T>&) >_func) : oldRV(_old), newRV(_new), func(_func) {}
		
		std::function< std::vector<R>(std::vector<T>&, RangeView<T>&) > func;
		RangeView<T> oldRV;
		RangeView<R> newRV;
	};

	template<typename T>
	std::vector<T> RangeView<T>::toVector() {
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

    //        if (count != -1) {
    //            result.resize(count);
    //        }

		return result;
	}

	template<typename T, typename F>
	RangeView<T> operator|(std::vector<T> &vec, Op<F> func) {
		RangeView<T> rv = RangeView<T>(vec);
		rv.addAction(std::function< std::vector<T>(std::vector<T>&, RangeView<T>&) >(func.getFunc()));

		return rv;
	}

	template<typename T, typename F>
	RangeView<T> operator|(std::vector<T> &vec, LazyTerminationOp<F> func) {
		RangeView<T> rv = RangeView<T>(vec);
        rv.addAction(std::function< std::vector<T>(std::vector<T>&, RangeView<T>&) >(func.getFunc()));

		return rv;
	}

	template<typename T, typename F>
	RangeView<T> operator|(RangeView<T> rv, Op<F> func) {
		rv.addAction(std::function< std::vector<T>(std::vector<T>&, RangeView<T>&) >(func.getFunc()));
		return rv;
	}

    template<typename T, typename F>
    auto operator|(RangeView<T> rv, TerminationOp<F> termOp) {
        auto newData = termOp.apply(std::vector<T>(), rv);
        using type = std::decay_t<decltype(newData[0])>;

        RangeViewBridge<T, type> bridge = RangeViewBridge<T, type>(rv, RangeView<type>(), std::function< std::vector<type>(std::vector<T>&, RangeView<T>&) >(termOp.getFunc()));
        return bridge;
    }

    template<typename T, typename F>
    auto operator|(RangeView<T> rv, LazyTerminationOp<F> termOp) {
    	if (rv.isEndless()) {
			termOp.apply(std::vector<T>(), rv);
		} else {
			rv.addAction(std::function< std::vector<T>(std::vector<T>&, RangeView<T>&) >(termOp.getFunc()));
		}

		return rv;
    }

    template<typename T, typename R, typename F>
    auto operator|(RangeViewBridge<T, R> bridge, LazyTerminationOp<F> termOp) {
    	if (bridge.oldRV.getCount() == -1) {
    		termOp.apply(std::vector<T>(), bridge.oldRV);
    	} 

    	bridge.newRV.addAction(std::function< std::vector<R>(std::vector<R>&, RangeView<R>&) >(termOp.getFunc()));
    	auto temp = bridge.oldRV.toVector();
    	auto data = bridge.func(temp, bridge.oldRV);

        bridge.newRV.setExtCollection(data);

    	return bridge.newRV;
    }

    template<typename T, typename R, typename F>
    auto operator|(RangeViewBridge<T, R> bridge, Op<F> op) {
        bridge.newRV.addAction(std::function< std::vector<R>(std::vector<R>&, RangeView<R>&) >(op.getFunc()));
        return bridge;
    };

	RangeView<int> ints(int n) {
		auto int_generator_func = [n](auto &v, auto &rv) {
			for (size_t i = n, count = 0; count < rv.getCount(); i++, count++) {
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
            if (rv.isEndless()) {
                rv.setCount(n);
            } else {
                rv.setCount(n);
                if (n < rv.getResultVec().size()) {
                    rv.getResultVec().resize(n);
                }
            }

			return v;
		};

        return LazyTerminationOp<decltype(take_func)>(take_func);
	}

	auto reverse() {
		auto reverse_func = [](auto &v, auto &rv) {
			std::reverse(v.begin(), v.end());
			return v;
		};

		return Op<decltype(reverse_func)>(reverse_func);
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

		return Op<decltype(remove_if_func)>(remove_if_func);
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