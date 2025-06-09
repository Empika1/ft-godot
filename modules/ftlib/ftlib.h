#ifndef FTLIB_H
#define FTLIB_H

#include "core/object/ref_counted.h"

class Summator : public RefCounted {
	GDCLASS(Summator, RefCounted);

	int count;

protected:
	static void _bind_methods();

public:
    void add_one();
	void add(int p_value);
	void reset();
	int get_total() const;

	Summator();
};

#endif // FTLIB_H