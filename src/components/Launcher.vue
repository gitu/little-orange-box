<script setup lang="ts">

import {onMounted, ref} from 'vue'

const connected = ref(false);
const launching = ref(false);
const launched = ref(false);


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
});

</script>

<template>
  <h1 v-if="!connected">
    PREPARE FOR LAUNCH!
  </h1>
  <h1 v-else-if="launched">
    LAUNCHED!
  </h1>
  <h1 v-else-if="launching" class="shake">
    LAUNCHING!
  </h1>
  <h1 v-else>
    READY FOR LAUNCH!
  </h1>
</template>

<style scoped>
.shake {
  animation: shake 0.82s cubic-bezier(0.36, 0.07, 0.19, 0.97) both;
  transform: translate3d(0, 0, 0);
}

@keyframes shake {
  10%,
  90% {
    transform: translate3d(-1px, 0, 0);
  }

  20%,
  80% {
    transform: translate3d(2px, 0, 0);
  }

  30%,
  50%,
  70% {
    transform: translate3d(-4px, 0, 0);
  }

  40%,
  60% {
    transform: translate3d(4px, 0, 0);
  }
}
</style>
