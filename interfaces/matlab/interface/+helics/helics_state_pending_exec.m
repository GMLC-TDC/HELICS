function v = helics_state_pending_exec()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 138);
  end
  v = vInitialized;
end
