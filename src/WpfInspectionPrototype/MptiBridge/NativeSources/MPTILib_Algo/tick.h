///////////////////////////////////////////////////////////////
// TimeCounter classes
// ∏∏ µÁ ¿Ã : ¿± ¡§ ªÔ
// ∏∏ µÁ ≥Ø : 2007.03.01
#pragma once

class tick
{
public:
	tick();
	virtual ~tick();

public:
	void	start();
	double	end();
	double	current();
	double	get() { return _result; }

protected:
	LARGE_INTEGER _start;
	LARGE_INTEGER _end;
	double	_result;

private:
	LARGE_INTEGER _freq;	
};
