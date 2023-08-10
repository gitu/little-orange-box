<script setup lang="ts">

import {onMounted, ref, watch} from 'vue'

const connected = ref(false);
const launching = ref(false);
const launched = ref(false);
const targetSet = ref(false);
const target = ref('https://youtu.be/dQw4w9WgXcQ');


onMounted(() => {
  console.log('mounted')
  window.addEventListener('gamepadconnected', (e) => {
    console.log(e)
    connected.value = true
  })
  window.addEventListener('gamepaddisconnected', (e) => {
    console.log(e)
    connected.value = false
  });
  setInterval(() => {
    launching.value = !!navigator.getGamepads()[0]?.buttons[0]?.pressed
    launched.value = !!navigator.getGamepads()[0]?.buttons[1]?.pressed
  }, 50)

  const queryString = window.location.search
  const urlParams = new URLSearchParams(queryString);
  if (urlParams.get("target")) {
    target.value = urlParams.get("target")!;
    targetSet.value = true;
    console.log('target set to' + target.value);
  }

});

watch(launched, (launched) => {
  if (launched && targetSet) {
    window.location.href = target.value;
  }
})

</script>

<template>
  <h1 v-if="launched">
    LAUNCHED!
  </h1>
  <h1 v-else-if="launching" class="shake">
    LAUNCHING!
  </h1>
  <h1 v-else-if="connected">
    READY FOR LAUNCH!
  </h1>
  <h1 v-else>
    PREPARE FOR LAUNCH!
  </h1>
</template>

<style scoped>
.shake {
  animation: tilt-n-move-shaking 0.35s infinite;
}

@keyframes tilt-n-move-shaking {
  0% { transform: translate(0, 0) rotate(0deg); }
  25% { transform: translate(5px, 5px) rotate(5deg); }
  50% { transform: translate(0, 0) rotate(0deg); }
  75% { transform: translate(-5px, 5px) rotate(-5deg); }
  100% { transform: translate(0, 0) rotate(0deg); }
}
</style>
