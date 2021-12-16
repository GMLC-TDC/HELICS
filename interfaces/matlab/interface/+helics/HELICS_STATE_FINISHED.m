function v = HELICS_STATE_FINISHED()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 149);
  end
  v = vInitialized;
end
