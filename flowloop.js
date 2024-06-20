const windSpeedSampleSize = 10; // Number of samples 
const windDirections = [0,22.5,45,67.5,90,112.5, 135, 157.5, 180, 202.5, 225, 247.5, 270, 292.5, 315, 337.5];
var windSpeedSamples = {};

windDirections.forEach (function(direction) { 
    windSpeedSamples[direction] = [];
    for (var sampleIndex = 0; sampleIndex < windSpeedSampleSize; sampleIndex++) {
        windSpeedSamples[direction][sampleIndex] = 0;
        console.log(sampleIndex);
    }
})
console.log(windSpeedSamples[0][0]);