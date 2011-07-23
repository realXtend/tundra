function add(inputname, change) {
    var form = document.forms[0]; //getElementById("camform");
    var input = form[inputname];
    var val = parseFloat(input.value);
    input.value = val + change;
    form.submit()
}