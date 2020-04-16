var now = new Date();
var min = now.getMinutes();
var startIn = 10 - (min % 10);
console.log('Start in ' + startIn + ' minutes');
setTimeout(runInterval, startIn * 60 * 1000);
function runInterval() {
    setInterval(function() {
        console.log('10 minute');
    }, 10 * 60 * 1000);
}