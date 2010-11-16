// The following code implements deep-copying of JavaScript built-in types.
// Booleans, integers and strings are value types in js, but
// arrays and objects act like reference types.
// The code below is taken from 
// http://my.opera.com/GreyWyvern/blog/show.dml/1725165

// See also e.g. this http://oranlooney.com/functional-javascript/ 

// To deep copy arrays, call the array.slice method with 0 as the first parameter.
// e.g.
//
// var foo = [1, 2, 3];
// var bar = foo.slice(0); // bar is now a deep copy of foo, not a reference.

// To deep copy Objects, call the object.clone() method, which we define
// below.

// Add the following prototype 'clone' to all JavaScript Objects. 
Object.prototype.clone = function() {
  var newObj = (this instanceof Array) ? [] : {};
  for (i in this) {
    if (i == 'clone') continue;
    if (this[i] && typeof this[i] == "object") {
      newObj[i] = this[i].clone();
    } else newObj[i] = this[i];
  } return newObj;
};
