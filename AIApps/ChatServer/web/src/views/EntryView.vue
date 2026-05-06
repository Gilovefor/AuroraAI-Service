<script setup>
import { reactive, ref } from 'vue'
import { useRouter } from 'vue-router'
import { login, register } from '../services/api'
import entryBg from '@aiapps-images/entry_pic.jpg'

const router = useRouter()
const mode = ref('login')
const loading = ref(false)
const message = ref('')

const form = reactive({
  username: '',
  password: ''
})

const shellStyle = {
  backgroundImage: `linear-gradient(180deg, rgba(0, 0, 0, 0.48) 0%, rgba(0, 0, 0, 0.62) 100%), url(${entryBg})`
}

async function submit() {
  if (!form.username || !form.password || loading.value) return
  loading.value = true
  message.value = ''

  try {
    if (mode.value === 'login') {
      const { response, data } = await login({ username: form.username, password: form.password })
      if (response.status === 200 && data?.userId) {
        sessionStorage.setItem('userId', String(data.userId))
        router.push('/menu')
        return
      }
      message.value = response.status === 401 ? '用户名或密码错误' : '登录失败，请稍后重试'
    } else {
      const { response } = await register({ username: form.username, password: form.password })
      if (response.status === 200) {
        message.value = '注册成功，请登录'
        mode.value = 'login'
        return
      }
      message.value = response.status === 409 ? '该用户名已存在' : '注册失败，请稍后重试'
    }
  } catch {
    message.value = '网络异常，请检查连接后重试'
  } finally {
    loading.value = false
  }
}
</script>

<template>
  <main class="entry-shell" :style="shellStyle">
    <div class="entry-scrim" aria-hidden="true" />
    <section class="entry-panel">
      <header class="entry-header">
        <h1 class="entry-title">AuroraAI</h1>
        <p class="entry-sub">AI 服务 · 登录</p>
      </header>

      <div class="entry-tabs" role="tablist">
        <button
          type="button"
          class="entry-tab"
          :class="{ active: mode === 'login' }"
          role="tab"
          :aria-selected="mode === 'login'"
          @click="mode = 'login'"
        >
          登录
        </button>
        <button
          type="button"
          class="entry-tab"
          :class="{ active: mode === 'register' }"
          role="tab"
          :aria-selected="mode === 'register'"
          @click="mode = 'register'"
        >
          注册
        </button>
      </div>

      <form class="entry-form" @submit.prevent="submit">
        <label class="entry-field">
          <span class="entry-label">用户名</span>
          <input v-model.trim="form.username" class="entry-input" autocomplete="username" placeholder="用户名" />
        </label>
        <label class="entry-field">
          <span class="entry-label">密码</span>
          <input
            v-model.trim="form.password"
            class="entry-input"
            type="password"
            autocomplete="current-password"
            placeholder="密码"
          />
        </label>
        <button class="entry-submit" type="submit" :disabled="loading">
          {{ loading ? '请稍候…' : mode === 'login' ? '进入' : '注册' }}
        </button>
      </form>

      <p v-if="message" class="entry-tip" role="status">{{ message }}</p>
    </section>
  </main>
</template>

<style scoped>
.entry-shell {
  position: relative;
  min-height: 100vh;
  min-height: 100dvh;
  width: 100%;
  margin: 0;
  padding: clamp(20px, 5vw, 40px);
  display: flex;
  align-items: center;
  justify-content: center;
  background-color: #0a0a0a;
  background-size: cover;
  background-position: center;
  background-repeat: no-repeat;
  box-sizing: border-box;
}

.entry-scrim {
  position: absolute;
  inset: 0;
  pointer-events: none;
  border-radius: 0;
  background: radial-gradient(ellipse 130% 90% at 50% 35%, transparent 0%, rgba(0, 0, 0, 0.38) 100%);
}

.entry-panel {
  position: relative;
  z-index: 1;
  width: min(400px, 100%);
  padding: 34px 30px 30px;
  border-radius: 22px;
  border: 1px solid rgba(255, 255, 255, 0.14);
  background: rgba(14, 14, 14, 0.58);
  backdrop-filter: blur(16px) saturate(120%);
  -webkit-backdrop-filter: blur(16px) saturate(120%);
  box-shadow:
    0 28px 56px rgba(0, 0, 0, 0.5),
    inset 0 1px 0 rgba(255, 255, 255, 0.06);
}

.entry-header {
  text-align: center;
  margin-bottom: 26px;
}

.entry-title {
  margin: 0;
  font-size: 1.5rem;
  font-weight: 600;
  letter-spacing: 0.18em;
  text-transform: uppercase;
  color: #fafafa;
}

.entry-sub {
  margin: 10px 0 0;
  font-size: 0.8125rem;
  letter-spacing: 0.18em;
  color: #9ca3af;
}

.entry-tabs {
  display: grid;
  grid-template-columns: 1fr 1fr;
  gap: 0;
  margin-bottom: 26px;
  padding: 4px;
  border-radius: 14px;
  border: 1px solid rgba(255, 255, 255, 0.1);
  background: rgba(0, 0, 0, 0.35);
  box-shadow: inset 0 1px 2px rgba(0, 0, 0, 0.35);
}

.entry-tab {
  margin: 0;
  padding: 11px 14px;
  font-size: 0.8125rem;
  font-weight: 500;
  letter-spacing: 0.1em;
  color: #9ca3af;
  background: transparent;
  border: none;
  border-radius: 11px;
  cursor: pointer;
  transition: color 0.16s ease, background 0.16s ease;
}

.entry-tab:hover {
  color: #e5e7eb;
}

.entry-tab.active {
  color: #fff;
  background: rgba(255, 255, 255, 0.1);
  box-shadow: 0 2px 8px rgba(0, 0, 0, 0.2);
}

.entry-form {
  display: grid;
  gap: 18px;
}

.entry-field {
  display: grid;
  gap: 7px;
}

.entry-label {
  font-size: 0.6875rem;
  font-weight: 600;
  letter-spacing: 0.12em;
  text-transform: uppercase;
  color: #6b7280;
}

.entry-input {
  width: 100%;
  box-sizing: border-box;
  padding: 12px 14px;
  font-size: 0.9375rem;
  color: #f3f4f6;
  background: rgba(0, 0, 0, 0.28);
  border: 1px solid rgba(255, 255, 255, 0.1);
  border-radius: 12px;
  outline: none;
  transition: border-color 0.16s ease, box-shadow 0.16s ease;
}

.entry-input::placeholder {
  color: rgba(107, 114, 128, 0.9);
}

.entry-input:focus {
  border-color: rgba(255, 255, 255, 0.28);
  box-shadow: 0 0 0 3px rgba(255, 255, 255, 0.06);
}

.entry-submit {
  margin-top: 6px;
  width: 100%;
  padding: 14px 20px;
  font-size: 0.75rem;
  font-weight: 600;
  letter-spacing: 0.18em;
  text-transform: uppercase;
  color: #0a0a0a;
  background: #e8e8e8;
  border: 1px solid #d4d4d4;
  border-radius: 14px;
  cursor: pointer;
  box-shadow: 0 6px 20px rgba(0, 0, 0, 0.25);
  transition: background 0.16s ease, transform 0.16s ease;
}

.entry-submit:hover:not(:disabled) {
  background: #fafafa;
  transform: translateY(-1px);
}

.entry-submit:disabled {
  opacity: 0.45;
  cursor: not-allowed;
  transform: none;
}

.entry-submit:focus-visible {
  outline: 2px solid #fff;
  outline-offset: 2px;
}

.entry-tip {
  margin: 18px 0 0;
  padding: 12px 14px;
  font-size: 0.8125rem;
  line-height: 1.5;
  color: #d1d5db;
  background: rgba(255, 255, 255, 0.05);
  border: 1px solid rgba(255, 255, 255, 0.1);
  border-radius: 12px;
}
</style>
