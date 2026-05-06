<script setup>
import { nextTick, onMounted, ref, watch } from 'vue'
import { useRouter } from 'vue-router'
import { getHistory, getSessions, sendMessage, sendMessageToNewSession } from '../services/api'

const router = useRouter()
const sessions = ref([])
const activeSessionId = ref('')
const draft = ref('')
const messages = ref([])
const modelType = ref('1')
const loading = ref(false)
const tempMode = ref(false)
const messageListEl = ref(null)

function asMessage(role, content) {
  return { role, content, id: Date.now() + Math.random() }
}

async function scrollMessagesToEnd() {
  await nextTick()
  const el = messageListEl.value
  if (el) el.scrollTop = el.scrollHeight
}

async function loadSessionList() {
  const { data } = await getSessions()
  if (data?.success && Array.isArray(data.sessions)) {
    sessions.value = data.sessions.map((s) => ({
      sessionId: String(s.sessionId),
      name: s.name || `会话 ${s.sessionId}`
    }))
  }
}

async function switchSession(id) {
  activeSessionId.value = String(id)
  tempMode.value = false
  messages.value = []
  const { data } = await getHistory(activeSessionId.value)
  if (data?.success && Array.isArray(data.history)) {
    messages.value = data.history.map((item) =>
      asMessage(item.is_user ? 'user' : 'assistant', item.content)
    )
  }
  await scrollMessagesToEnd()
}

function createTempSession() {
  tempMode.value = true
  activeSessionId.value = ''
  messages.value = []
}

async function submit() {
  const question = draft.value.trim()
  if (!question || loading.value) return

  loading.value = true
  messages.value.push(asMessage('user', question))
  await scrollMessagesToEnd()

  try {
    if (tempMode.value) {
      const { data } = await sendMessageToNewSession({ question, modelType: modelType.value })
      if (data?.success) {
        const sid = String(data.sessionId)
        activeSessionId.value = sid
        tempMode.value = false
        messages.value.push(asMessage('assistant', data.Information || ''))
        await loadSessionList()
      } else {
        messages.value.push(asMessage('assistant', data?.message || '无法创建会话'))
      }
    } else if (activeSessionId.value) {
      const { data } = await sendMessage({
        question,
        modelType: modelType.value,
        sessionId: activeSessionId.value
      })
      messages.value.push(asMessage('assistant', data?.success ? data.Information : (data?.message || '发送失败')))
    } else {
      messages.value.push(asMessage('assistant', '请先选择会话或点击「新建」'))
    }
  } catch {
    messages.value.push(asMessage('assistant', '请求失败，请稍后重试'))
  } finally {
    draft.value = ''
    loading.value = false
    await scrollMessagesToEnd()
  }
}

watch(
  () => messages.value.length,
  () => scrollMessagesToEnd()
)

onMounted(async () => {
  await loadSessionList()
  await scrollMessagesToEnd()
})
</script>

<template>
  <main class="chat-shell">
    <aside class="chat-sidebar">
      <div class="sidebar-top">
        <div>
          <p class="lbl">会话</p>
          <p class="sidebar-heading">列表</p>
        </div>
        <button type="button" class="btn-pill" @click="createTempSession">新建</button>
      </div>
      <div class="sidebar-scroll">
        <button
          v-for="item in sessions"
          :key="item.sessionId"
          type="button"
          class="session-pill"
          :class="{ active: activeSessionId === item.sessionId && !tempMode }"
          @click="switchSession(item.sessionId)"
        >
          <span class="dot" aria-hidden="true" />
          <span class="session-text">{{ item.name }}</span>
        </button>
        <p v-if="!sessions.length" class="sidebar-empty">暂无会话，点击「新建」开始</p>
      </div>
    </aside>

    <section class="chat-body">
      <header class="body-top">
        <div class="top-cluster">
          <button type="button" class="btn-outline" @click="router.push('/menu')">返回</button>
          <div class="top-text">
            <p class="lbl">对话</p>
            <p class="state">
              {{
                tempMode
                  ? '新会话（首条消息后将保存）'
                  : activeSessionId
                    ? `当前 · ${activeSessionId.slice(0, 10)}…`
                    : '请选择左侧会话'
              }}
            </p>
          </div>
        </div>
        <label class="model-field">
          <span class="model-lbl">模式</span>
          <select v-model="modelType" class="model-select">
            <option value="1">通用问答</option>
            <option value="2">联网搜索</option>
            <option value="3">知识 RAG</option>
            <option value="4">Tool MCP</option>
          </select>
        </label>
      </header>

      <div ref="messageListEl" class="msg-scroll">
        <div v-for="m in messages" :key="m.id" class="msg-line" :class="m.role">
          <div class="bubble" :class="m.role">
            <span class="tag">{{ m.role === 'user' ? '我' : 'AI' }}</span>
            <p class="text">{{ m.content }}</p>
          </div>
        </div>
      </div>

      <form class="composer" @submit.prevent="submit">
        <textarea
          v-model="draft"
          class="composer-input"
          rows="2"
          placeholder="输入消息…"
          @keydown.enter.exact.prevent="submit"
        />
        <button class="composer-btn" type="submit" :disabled="loading">
          {{ loading ? '…' : '发送' }}
        </button>
      </form>
    </section>
  </main>
</template>

<style scoped>
.chat-shell {
  --edge: rgba(255, 255, 255, 0.1);
  --muted: #9ca3af;

  height: 100vh;
  height: 100dvh;
  width: 100%;
  margin: 0;
  padding: 10px;
  box-sizing: border-box;
  display: flex;
  flex-direction: row;
  gap: 10px;
  background: #080808;
  color: #f3f4f6;
  overflow: hidden;
}

.chat-sidebar {
  width: min(272px, 32vw);
  flex-shrink: 0;
  display: flex;
  flex-direction: column;
  min-height: 0;
  border-radius: 20px;
  border: 1px solid var(--edge);
  background: rgba(14, 14, 14, 0.95);
  box-shadow: inset 0 1px 0 rgba(255, 255, 255, 0.04);
  overflow: hidden;
}

.sidebar-top {
  flex-shrink: 0;
  display: flex;
  align-items: flex-start;
  justify-content: space-between;
  gap: 10px;
  padding: 16px 14px 12px;
  border-bottom: 1px solid var(--edge);
}

.lbl {
  margin: 0;
  font-size: 0.625rem;
  font-weight: 600;
  letter-spacing: 0.14em;
  text-transform: uppercase;
  color: #6b7280;
}

.sidebar-heading {
  margin: 5px 0 0;
  font-size: 0.875rem;
  font-weight: 600;
}

.btn-pill {
  padding: 7px 12px;
  font-size: 0.625rem;
  font-weight: 600;
  letter-spacing: 0.1em;
  text-transform: uppercase;
  color: #e5e7eb;
  background: rgba(255, 255, 255, 0.06);
  border: 1px solid rgba(255, 255, 255, 0.12);
  border-radius: 999px;
  cursor: pointer;
  transition: background 0.16s ease, border-color 0.16s ease;
}

.btn-pill:hover {
  background: rgba(255, 255, 255, 0.1);
  border-color: rgba(255, 255, 255, 0.2);
}

.sidebar-scroll {
  flex: 1;
  min-height: 0;
  overflow-y: auto;
  padding: 12px 10px 14px;
}

.sidebar-scroll::-webkit-scrollbar {
  width: 6px;
}

.sidebar-scroll::-webkit-scrollbar-thumb {
  background: rgba(255, 255, 255, 0.12);
  border-radius: 99px;
}

.session-pill {
  width: 100%;
  display: flex;
  align-items: center;
  gap: 10px;
  margin-bottom: 8px;
  padding: 11px 12px;
  text-align: left;
  border-radius: 14px;
  border: 1px solid transparent;
  background: rgba(255, 255, 255, 0.04);
  color: inherit;
  cursor: pointer;
  transition: background 0.15s ease, border-color 0.15s ease;
}

.session-pill:hover {
  background: rgba(255, 255, 255, 0.07);
}

.session-pill.active {
  border-color: rgba(255, 255, 255, 0.18);
  background: rgba(255, 255, 255, 0.1);
  box-shadow: inset 0 1px 0 rgba(255, 255, 255, 0.06);
}

.dot {
  width: 7px;
  height: 7px;
  border-radius: 50%;
  background: #6b7280;
  flex-shrink: 0;
}

.session-pill.active .dot {
  background: #e5e7eb;
  box-shadow: 0 0 0 3px rgba(255, 255, 255, 0.08);
}

.session-text {
  font-size: 0.8125rem;
  overflow: hidden;
  text-overflow: ellipsis;
  white-space: nowrap;
  color: #d1d5db;
}

.sidebar-empty {
  margin: 18px 8px;
  font-size: 0.8125rem;
  line-height: 1.5;
  text-align: center;
  color: var(--muted);
}

.chat-body {
  flex: 1;
  min-width: 0;
  min-height: 0;
  display: flex;
  flex-direction: column;
  border-radius: 20px;
  border: 1px solid var(--edge);
  background: rgba(12, 12, 12, 0.98);
  box-shadow: inset 0 1px 0 rgba(255, 255, 255, 0.04);
  overflow: hidden;
}

.body-top {
  flex-shrink: 0;
  display: flex;
  align-items: center;
  justify-content: space-between;
  gap: 12px;
  flex-wrap: wrap;
  padding: 14px 18px;
  border-bottom: 1px solid var(--edge);
}

.top-cluster {
  display: flex;
  align-items: center;
  gap: 14px;
  min-width: 0;
}

.btn-outline {
  padding: 8px 14px;
  font-size: 0.6875rem;
  font-weight: 600;
  letter-spacing: 0.08em;
  text-transform: uppercase;
  color: #d1d5db;
  background: transparent;
  border: 1px solid rgba(255, 255, 255, 0.14);
  border-radius: 12px;
  cursor: pointer;
  transition: border-color 0.15s ease, background 0.15s ease;
}

.btn-outline:hover {
  border-color: rgba(255, 255, 255, 0.26);
  background: rgba(255, 255, 255, 0.05);
}

.top-text {
  min-width: 0;
}

.state {
  margin: 4px 0 0;
  font-size: 0.75rem;
  color: var(--muted);
  overflow: hidden;
  text-overflow: ellipsis;
  white-space: nowrap;
  max-width: min(400px, 48vw);
}

.model-field {
  display: flex;
  align-items: center;
  gap: 10px;
}

.model-lbl {
  font-size: 0.625rem;
  font-weight: 600;
  letter-spacing: 0.12em;
  text-transform: uppercase;
  color: #6b7280;
}

.model-select {
  min-width: 132px;
  padding: 9px 30px 9px 12px;
  font-size: 0.8125rem;
  color: #e5e7eb;
  background: rgba(0, 0, 0, 0.4);
  border: 1px solid var(--edge);
  border-radius: 12px;
  cursor: pointer;
  appearance: none;
  background-image: url("data:image/svg+xml,%3Csvg xmlns='http://www.w3.org/2000/svg' width='10' height='10' viewBox='0 0 12 12'%3E%3Cpath fill='%239ca3af' d='M3 4.5L6 8l3-3.5'/%3E%3C/svg%3E");
  background-repeat: no-repeat;
  background-position: right 10px center;
}

.model-select:focus {
  outline: none;
  border-color: rgba(255, 255, 255, 0.25);
  box-shadow: 0 0 0 3px rgba(255, 255, 255, 0.06);
}

.msg-scroll {
  flex: 1;
  min-height: 0;
  overflow-y: auto;
  padding: 16px 18px;
  scroll-behavior: smooth;
}

.msg-scroll::-webkit-scrollbar {
  width: 8px;
}

.msg-scroll::-webkit-scrollbar-thumb {
  background: rgba(255, 255, 255, 0.1);
  border-radius: 99px;
}

.msg-line {
  display: flex;
  margin-bottom: 14px;
}

.msg-line.user {
  justify-content: flex-end;
}

.msg-line.assistant {
  justify-content: flex-start;
}

.bubble {
  max-width: min(82%, 640px);
  padding: 13px 16px;
  border: 1px solid var(--edge);
  border-radius: 18px;
}

.bubble.user {
  background: rgba(28, 28, 28, 0.95);
  border-bottom-right-radius: 6px;
  box-shadow: 0 4px 16px rgba(0, 0, 0, 0.25);
}

.bubble.assistant {
  background: rgba(20, 20, 20, 0.95);
  border-bottom-left-radius: 6px;
}

.tag {
  display: block;
  font-size: 0.625rem;
  font-weight: 700;
  letter-spacing: 0.12em;
  color: #6b7280;
  margin-bottom: 8px;
}

.text {
  margin: 0;
  font-size: 0.9375rem;
  line-height: 1.55;
  white-space: pre-wrap;
  color: #e5e7eb;
}

.composer {
  flex-shrink: 0;
  display: grid;
  grid-template-columns: 1fr auto;
  gap: 12px;
  align-items: end;
  margin: 0 12px 12px;
  padding: 12px 14px;
  border-radius: 16px;
  border: 1px solid var(--edge);
  background: rgba(0, 0, 0, 0.35);
  box-shadow: inset 0 1px 0 rgba(255, 255, 255, 0.04);
}

.composer-input {
  width: 100%;
  min-height: 48px;
  max-height: 140px;
  resize: vertical;
  padding: 10px 4px;
  font-size: 0.9375rem;
  line-height: 1.45;
  color: #f3f4f6;
  background: transparent;
  border: none;
  border-bottom: 1px solid rgba(255, 255, 255, 0.15);
  border-radius: 0;
  outline: none;
  font-family: inherit;
}

.composer-input::placeholder {
  color: #6b7280;
}

.composer-input:focus {
  border-bottom-color: rgba(255, 255, 255, 0.4);
}

.composer-btn {
  padding: 12px 22px;
  font-size: 0.6875rem;
  font-weight: 600;
  letter-spacing: 0.14em;
  text-transform: uppercase;
  color: #0a0a0a;
  background: #e5e5e5;
  border: 1px solid #d4d4d4;
  border-radius: 14px;
  cursor: pointer;
  align-self: stretch;
  box-shadow: 0 4px 14px rgba(0, 0, 0, 0.2);
  transition: background 0.15s ease;
}

.composer-btn:hover:not(:disabled) {
  background: #fafafa;
}

.composer-btn:disabled {
  opacity: 0.4;
  cursor: not-allowed;
}

@media (max-width: 768px) {
  .chat-shell {
    flex-direction: column;
    padding: 8px;
  }

  .chat-sidebar {
    width: 100%;
    max-height: 34vh;
  }

  .state {
    max-width: 100%;
  }

  .composer {
    grid-template-columns: 1fr;
    margin: 0 8px 8px;
  }

  .composer-btn {
    min-height: 44px;
  }

  .model-field {
    width: 100%;
  }

  .model-select {
    flex: 1;
    max-width: none;
  }
}
</style>
