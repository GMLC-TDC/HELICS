function v = next_step()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 795176316);
  end
  v = vInitialized;
end
