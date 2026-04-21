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
  () => {
    scrollMessagesToEnd()
  }
)

onMounted(async () => {
  await loadSessionList()
  await scrollMessagesToEnd()
})
</script>

<template>
  <main class="chat-shell">
    <aside class="chat-sidebar">
      <div class="sidebar-head">
        <div>
          <p class="lbl">会话</p>
          <p class="sidebar-title">列表</p>
        </div>
        <button type="button" class="btn-ghost sm" @click="createTempSession">新建</button>
      </div>
      <div class="session-scroll">
        <button
          v-for="item in sessions"
          :key="item.sessionId"
          type="button"
          class="session-row"
          :class="{ active: activeSessionId === item.sessionId && !tempMode }"
          @click="switchSession(item.sessionId)"
        >
          <span class="session-line" aria-hidden="true" />
          <span class="session-label">{{ item.name }}</span>
        </button>
        <p v-if="!sessions.length" class="session-empty">暂无会话，点击「新建」开始</p>
      </div>
    </aside>

    <section class="chat-main">
      <header class="chat-top">
        <div class="top-left">
          <button type="button" class="btn-ghost" @click="router.push('/menu')">返回</button>
          <div class="top-meta">
            <p class="lbl">对话</p>
            <p class="state-line">
              {{
                tempMode
                  ? '新会话（未提交前未保存）'
                  : activeSessionId
                    ? `当前 · ${activeSessionId.slice(0, 10)}…`
                    : '请选择左侧会话'
              }}
            </p>
          </div>
        </div>
        <label class="model-box">
          <span class="model-tag">模式</span>
          <select v-model="modelType" class="model-select">
            <option value="1">通用问答</option>
            <option value="2">联网搜索</option>
            <option value="3">知识 RAG</option>
            <option value="4">Tool MCP</option>
          </select>
        </label>
      </header>

      <div ref="messageListEl" class="message-scroll">
        <div v-for="m in messages" :key="m.id" class="msg-row" :class="m.role">
          <div class="bubble" :class="m.role">
            <span class="bubble-tag">{{ m.role === 'user' ? '我' : 'AI' }}</span>
            <p class="bubble-text">{{ m.content }}</p>
          </div>
        </div>
      </div>

      <form class="composer" @submit.prevent="submit">
        <textarea
          v-model="draft"
          class="composer-area"
          rows="2"
          placeholder="输入消息…"
          @keydown.enter.exact.prevent="submit"
        />
        <button class="composer-send" type="submit" :disabled="loading">
          {{ loading ? '…' : '发送' }}
        </button>
      </form>
    </section>
  </main>
</template>

<style scoped>
.chat-shell {
  --line: rgba(255, 255, 255, 0.1);
  --muted: #9ca3af;
  --paper: #111111;

  height: 100vh;
  height: 100dvh;
  width: 100%;
  margin: 0;
  box-sizing: border-box;
  display: flex;
  flex-direction: row;
  background: #0a0a0a;
  color: #f3f4f6;
  overflow: hidden;
}

.chat-sidebar {
  width: min(280px, 34vw);
  flex-shrink: 0;
  display: flex;
  flex-direction: column;
  min-height: 0;
  border-right: 1px solid var(--line);
  background: #0c0c0c;
}

.sidebar-head {
  flex-shrink: 0;
  display: flex;
  align-items: flex-start;
  justify-content: space-between;
  gap: 12px;
  padding: 16px 14px 12px;
  border-bottom: 1px solid var(--line);
}

.lbl {
  margin: 0;
  font-size: 0.625rem;
  font-weight: 600;
  letter-spacing: 0.14em;
  text-transform: uppercase;
  color: #6b7280;
}

.sidebar-title {
  margin: 4px 0 0;
  font-size: 0.8125rem;
  font-weight: 600;
  letter-spacing: 0.1em;
}

.btn-ghost {
  padding: 8px 12px;
  font-size: 0.6875rem;
  font-weight: 600;
  letter-spacing: 0.12em;
  text-transform: uppercase;
  color: #d1d5db;
  background: transparent;
  border: 1px solid rgba(255, 255, 255, 0.14);
  cursor: pointer;
  transition: border-color 0.15s ease, background 0.15s ease;
}

.btn-ghost:hover {
  border-color: rgba(255, 255, 255, 0.28);
  background: rgba(255, 255, 255, 0.05);
}

.btn-ghost.sm {
  padding: 6px 10px;
  font-size: 0.625rem;
}

.session-scroll {
  flex: 1;
  min-height: 0;
  overflow-y: auto;
  padding: 10px 10px 14px;
}

.session-scroll::-webkit-scrollbar {
  width: 6px;
}

.session-scroll::-webkit-scrollbar-thumb {
  background: rgba(255, 255, 255, 0.12);
  border-radius: 99px;
}

.session-row {
  width: 100%;
  display: flex;
  align-items: center;
  gap: 10px;
  margin-bottom: 6px;
  padding: 10px 10px;
  text-align: left;
  border: 1px solid transparent;
  background: rgba(255, 255, 255, 0.03);
  color: inherit;
  cursor: pointer;
  transition: border-color 0.15s ease, background 0.15s ease;
}

.session-row:hover {
  background: rgba(255, 255, 255, 0.06);
}

.session-row.active {
  border-color: rgba(255, 255, 255, 0.2);
  background: rgba(255, 255, 255, 0.08);
}

.session-line {
  width: 2px;
  height: 1rem;
  background: #4b5563;
  flex-shrink: 0;
}

.session-row.active .session-line {
  background: #e5e7eb;
}

.session-label {
  font-size: 0.8125rem;
  overflow: hidden;
  text-overflow: ellipsis;
  white-space: nowrap;
  color: #d1d5db;
}

.session-empty {
  margin: 20px 8px;
  font-size: 0.8125rem;
  line-height: 1.5;
  text-align: center;
  color: var(--muted);
}

.chat-main {
  flex: 1;
  min-width: 0;
  min-height: 0;
  display: flex;
  flex-direction: column;
  background: var(--paper);
}

.chat-top {
  flex-shrink: 0;
  display: flex;
  align-items: center;
  justify-content: space-between;
  gap: 12px;
  flex-wrap: wrap;
  padding: 12px 16px;
  border-bottom: 1px solid var(--line);
}

.top-left {
  display: flex;
  align-items: center;
  gap: 14px;
  min-width: 0;
}

.top-meta {
  min-width: 0;
}

.state-line {
  margin: 4px 0 0;
  font-size: 0.75rem;
  color: var(--muted);
  overflow: hidden;
  text-overflow: ellipsis;
  white-space: nowrap;
  max-width: min(420px, 50vw);
}

.model-box {
  display: flex;
  align-items: center;
  gap: 10px;
}

.model-tag {
  font-size: 0.625rem;
  font-weight: 600;
  letter-spacing: 0.12em;
  text-transform: uppercase;
  color: #6b7280;
}

.model-select {
  min-width: 128px;
  max-width: 200px;
  padding: 8px 28px 8px 10px;
  font-size: 0.8125rem;
  color: #e5e7eb;
  background: rgba(0, 0, 0, 0.4);
  border: 1px solid var(--line);
  border-radius: 0;
  cursor: pointer;
  appearance: none;
  background-image: url("data:image/svg+xml,%3Csvg xmlns='http://www.w3.org/2000/svg' width='10' height='10' viewBox='0 0 12 12'%3E%3Cpath fill='%239ca3af' d='M3 4.5L6 8l3-3.5'/%3E%3C/svg%3E");
  background-repeat: no-repeat;
  background-position: right 8px center;
}

.model-select:focus {
  outline: 1px solid rgba(255, 255, 255, 0.35);
  outline-offset: 1px;
}

.message-scroll {
  flex: 1;
  min-height: 0;
  overflow-y: auto;
  padding: 16px 18px;
  scroll-behavior: smooth;
}

.message-scroll::-webkit-scrollbar {
  width: 8px;
}

.message-scroll::-webkit-scrollbar-thumb {
  background: rgba(255, 255, 255, 0.1);
  border-radius: 99px;
}

.msg-row {
  display: flex;
  margin-bottom: 14px;
}

.msg-row.user {
  justify-content: flex-end;
}

.msg-row.assistant {
  justify-content: flex-start;
}

.bubble {
  max-width: min(82%, 640px);
  padding: 12px 14px;
  border: 1px solid var(--line);
}

.bubble.user {
  background: #1a1a1a;
  border-bottom-right-radius: 2px;
}

.bubble.assistant {
  background: #141414;
  border-bottom-left-radius: 2px;
}

.bubble-tag {
  display: block;
  font-size: 0.625rem;
  font-weight: 700;
  letter-spacing: 0.14em;
  color: #6b7280;
  margin-bottom: 8px;
}

.bubble-text {
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
  padding: 12px 16px 16px;
  border-top: 1px solid var(--line);
  background: #0c0c0c;
}

.composer-area {
  width: 100%;
  min-height: 52px;
  max-height: 160px;
  resize: vertical;
  padding: 12px 0 10px;
  font-size: 0.9375rem;
  line-height: 1.45;
  color: #f3f4f6;
  background: transparent;
  border: none;
  border-bottom: 1px solid rgba(255, 255, 255, 0.18);
  border-radius: 0;
  outline: none;
  font-family: inherit;
}

.composer-area::placeholder {
  color: #6b7280;
}

.composer-area:focus {
  border-bottom-color: rgba(255, 255, 255, 0.45);
}

.composer-send {
  padding: 12px 22px;
  font-size: 0.6875rem;
  font-weight: 600;
  letter-spacing: 0.16em;
  text-transform: uppercase;
  color: #0a0a0a;
  background: #e5e5e5;
  border: 1px solid #d4d4d4;
  cursor: pointer;
  transition: background 0.15s ease;
  align-self: stretch;
}

.composer-send:hover:not(:disabled) {
  background: #fafafa;
}

.composer-send:disabled {
  opacity: 0.4;
  cursor: not-allowed;
}

@media (max-width: 768px) {
  .chat-shell {
    flex-direction: column;
  }

  .chat-sidebar {
    width: 100%;
    max-height: 36vh;
    border-right: none;
    border-bottom: 1px solid var(--line);
  }

  .state-line {
    max-width: 100%;
  }

  .composer {
    grid-template-columns: 1fr;
  }

  .composer-send {
    min-height: 44px;
  }

  .model-box {
    width: 100%;
  }

  .model-select {
    flex: 1;
    max-width: none;
  }
}
</style>
