function v = helics_pending_init_state()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 795176325);
  end
  v = vInitialized;
end
