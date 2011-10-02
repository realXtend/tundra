
/// From Irrlicht.
///\todo Remove when our own Math lib has equivalent.
/// returns linear interpolation of a and b with ratio t
/// @return: a if t==0, b if t==1, and the linear interpolation else
template<class T>
inline T lerp(const T& a, const T& b, const f32 t)
{
	return (T)(a*(1.f-t)) + (b*t);
}
