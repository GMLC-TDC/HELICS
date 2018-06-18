function v = helics_finalize_state()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 795176323);
  end
  v = vInitialized;
end
