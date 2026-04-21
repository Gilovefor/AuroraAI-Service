<script setup>
import { useRouter } from 'vue-router'
const router = useRouter()
</script>

<template>
  <main class="page-shell notfound-wrap">
    <section class="card box">
      <p class="code" aria-hidden="true">404</p>
      <h1>页面不存在</h1>
      <p class="muted lead">链接可能已失效，或地址输入有误。</p>
      <button type="button" class="button" @click="router.push('/')">返回首页</button>
      <button type="button" class="button secondary ghost" @click="router.back()">上一页</button>
    </section>
  </main>
</template>

<style scoped>
.notfound-wrap {
  display: grid;
  place-items: center;
  padding-top: clamp(32px, 10vh, 80px);
}

.box {
  width: min(440px, 92vw);
  padding: 36px 28px 32px;
  text-align: center;
  position: relative;
  overflow: hidden;
}

.box::before {
  content: '';
  position: absolute;
  inset: -40% -20%;
  background: radial-gradient(circle, rgba(125, 162, 255, 0.12), transparent 55%);
  pointer-events: none;
}

.code {
  margin: 0;
  font-size: clamp(3.5rem, 14vw, 5rem);
  font-weight: 800;
  letter-spacing: -0.06em;
  line-height: 1;
  background: linear-gradient(135deg, #fff 0%, var(--accent) 45%, var(--accent-2) 100%);
  -webkit-background-clip: text;
  background-clip: text;
  color: transparent;
  position: relative;
}

h1 {
  margin: 18px 0 0;
  font-size: 1.25rem;
  font-weight: 700;
  position: relative;
}

.lead {
  margin: 10px 0 22px;
  line-height: 1.55;
  position: relative;
}

.box .button {
  position: relative;
  min-width: 160px;
}

.ghost {
  margin-top: 10px;
  width: 100%;
  max-width: 280px;
  margin-left: auto;
  margin-right: auto;
  display: block;
}
</style>